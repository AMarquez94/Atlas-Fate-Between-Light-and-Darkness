//--------------------------------------------------------------------------------------
#include "pbr.fx"

static const int MAX_RAY_STEPS = 40;
static const int MAX_BIN_STEPS = 10;
static const float RAY_STEP = 0.25f;

float4 BinarySearch(inout float3 reflected_ray, float3 hit_coord)
{
    float depth = 0;
    float depth_diff = 0;
    float4 projectedCoord;

    for (int i = 0; i < MAX_BIN_STEPS; i++)
    {
        projectedCoord = mul(float4(hit_coord, 1.f), camera_proj);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
        depth = txGBufferLinearDepth.Load(uint3(projectedCoord.xy / camera_inv_resolution, 0)).x;
        float3 worldCoords = getWorldCoords(projectedCoord.xy / camera_inv_resolution, depth);
        depth = mul(float4(worldCoords, 1.f), camera_view).z;
        depth_diff = hit_coord.z - depth;

        reflected_ray *= 0.5;
        hit_coord += reflected_ray * ((depth_diff > 0.f) - (depth_diff < 0.f));
    }

    projectedCoord = mul(float4(hit_coord, 1.f), camera_proj);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    depth = txGBufferLinearDepth.Load(uint3(projectedCoord.xy / camera_inv_resolution, 0)).x;
    float3 worldCoords = getWorldCoords(projectedCoord.xy / camera_inv_resolution, depth);
    depth = mul(float4(worldCoords, 1.f), camera_view).z;
    depth_diff = hit_coord.z - depth;

    return float4(projectedCoord.xy, depth, 1.0) * (abs(depth_diff) < 0.05 ? 1.f : 0.f);
}

float4 RayMarching(float3 reflected_ray, float3 hit_coord)
{
    float depth = 0;
    float depth_diff = 0;
    float4 projectedCoord;
    reflected_ray *= RAY_STEP;

    for (int i = 0; i < MAX_RAY_STEPS; i++)
    {
        hit_coord += reflected_ray;
        projectedCoord = mul(float4(hit_coord, 1.f), camera_proj);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
        depth = txGBufferLinearDepth.Load(uint3(projectedCoord.xy / camera_inv_resolution, 0)).x;
        float3 wPos = getWorldCoords(projectedCoord.xy / camera_inv_resolution, depth);
        depth = mul(float4(wPos, 1.f), camera_view).z;

        if (projectedCoord.x < 0 || projectedCoord.x > 1 || projectedCoord.y < 0 || projectedCoord.y > 1)
            return float4(0, 0, 0, 0);

        depth_diff = (hit_coord.z - depth);

        if (depth_diff < 0.0) {
            return BinarySearch(reflected_ray, hit_coord);
            return float4(projectedCoord.xy, depth, 1.0);
        }
    }

    return float4(projectedCoord.xy, depth, 0);
}

float4 PS(in float4 iPosition : SV_POSITION, in float2 iTex0 : TEXCOORD0) : SV_Target
{
    float4 color = txAlbedo.Sample(samClampLinear, iTex0);
		return color;
    int3 ss_load_coords = uint3(iPosition.xy, 0);

    // Decode GBuffer information
    float  roughness;
    float3 wPos, N, albedo, specular_color, reflected_dir, view_dir;

    decodeGBuffer(iPosition.xy, wPos, N, albedo, specular_color, roughness, reflected_dir, view_dir);
    float3 env_fresnel = Specular_F_Roughness(specular_color, 1. - pow(roughness, 2), N, view_dir);

    float4 view_normal = mul(float4(N, 0), camera_view);
    float4 view_pos = mul(float4(wPos, 1), camera_view);
    float3 reflected = normalize(reflect(view_pos, view_normal).xyz);

    float4 coords = RayMarching(reflected , view_pos);

    float2 d_coords = float2(1, 1) - pow(saturate(abs(coords.xy - float2(0.5f, 0.5f)) * 4), 2);
    float edge_factor = saturate(min(d_coords.x, d_coords.y));
    float multiplier = clamp(edge_factor * saturate(-reflected.z), 0, 0.9);
    float depth = txGBufferLinearDepth.Load(ss_load_coords).x;

    float4 ssr = txAlbedo.SampleLevel(samClampLinear, coords.xy, 0);
    float alpha = multiplier * float4(env_fresnel, coords.w) * (depth > 0.99 ? 0 : 1);

    return ((1 - alpha) * color) + ssr * alpha;
}



/*
Sample from McGuire. Tried to improve the results but were not good enough


static const float CB_STRIDE = 2.0f;
static const float CB_MAXDISTANCE = 1000.0f;
static const int CB_MAXSTEPS = 20;
static const float CB_STRIDECUTOFF = 1.0f;
static const float CB_ZTHICK = 1.0f;


float distanceSquared(float2 a, float2 b)
{
        a -= b;
        return dot(a, a);
}

bool intersectsDepthBuffer(float z, float minZ, float maxZ)
{
        float depthScale = min(1.0f, z * CB_STRIDECUTOFF);
        z += CB_ZTHICK + lerp(0.0f, 2.0f, depthScale);
        return (maxZ >= z) && (minZ - CB_ZTHICK <= z);
}

bool rayIntersectsDepthBuffer (float minZ, float maxZ, float2 depthUV) {
    float z = txGBufferLinearDepth.Load(uint3(depthUV, 0)).x;

    float depthScale = min(1.0f, z * CB_STRIDECUTOFF);
    z += CB_ZTHICK + lerp(0.0f, 2.0f, depthScale);
    return (maxZ >= z) && (minZ - CB_ZTHICK <= z);
}

void swap(inout float a, inout float b)
{
        float t = a;
        a = b;
        b = t;
}


// Returns true if the ray hit something
bool traceScreenSpaceRay(float3 csOrig, float3 csDir, float jitter, out float2 hitPixel, out float3 hitPoint)
{
        float rayLength = ((csOrig.z + csDir.z * CB_MAXDISTANCE) < camera_znear) ? (camera_znear - csOrig.z) / csDir.z : CB_MAXDISTANCE;
        float3 csEndPoint = csOrig + csDir * rayLength;

        // Project into homogeneous clip space
        float4 H0 = mul(float4(csOrig, 1.0f), camera_proj);
        float4 H1 = mul(float4(csEndPoint, 1.0f), camera_proj);
        float k0 = (1.0f / H0.w);
        float k1 = (1.0f / H1.w);

        // The interpolated homogeneous version of the camera-space points
        float3 Q0 = csOrig * k0;
        float3 Q1 = csEndPoint * k1;

        // Screen-space endpoints
        float2 P0 = ((H0.xy * k0) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f)) / camera_inv_resolution;
        float2 P1 = ((H1.xy * k1) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f)) / camera_inv_resolution;

        P1 += (distanceSquared(P0, P1) < 0.0001f) ? float2(0.01f, 0.01f) : 0.0f;
        float2 delta = P1 - P0;

        bool permute = false;
        if(abs(delta.x) < abs(delta.y))
        {
                permute = true;
                delta = delta.yx;
                P0 = P0.yx;
                P1 = P1.yx;
        }

        float stepDir = sign(delta.x);
        float invdx = stepDir / delta.x;

        float3 dQ = (Q1 - Q0) * invdx;
        float dk = (k1 - k0) * invdx;
        float2 dP = float2(stepDir, delta.y * invdx);


        float strideScale = 1.0f - min(1.0f, csOrig.z * CB_STRIDECUTOFF);
        float stride = 1.0f + strideScale * CB_STRIDE;
        dP *= stride;
        dQ *= stride;
        dk *= stride;

        P0 += dP * jitter;
        Q0 += dQ * jitter;
        k0 += dk * jitter;

        float4 PQk = float4(P0, Q0.z, k0);
        float4 dPQk = float4(dP, dQ.z, dk);
        float3 Q = Q0;

        // Adjust end condition for iteration direction
        float end = P1.x * stepDir;

        float stepCount = 0.0f;
        float prevZMaxEstimate = csOrig.z;
        float rayZMin = prevZMaxEstimate;
        float rayZMax = prevZMaxEstimate;
        float sceneZMax = rayZMax + 100.0f;
        float intersect = false;
        float i= 0;

        for( i=0; i<100 && intersect == false; i++)
        {
                PQk += dPQk;
                rayZMin = prevZMaxEstimate;
                rayZMax = (dPQk.z * 0.5f + PQk.z) / (dPQk.w * 0.5f + PQk.w);
                prevZMaxEstimate = rayZMax;
                if(rayZMin > rayZMax)
                {
                    swap(rayZMin, rayZMax);
                }

                hitPixel = permute ? PQk.yx : PQk.xy;
                intersect = intersectsDepthBuffer(rayZMin, rayZMax, hitPixel);
        }

        // Advance Q based on the number of steps
        Q.xy += dQ.xy * stepCount;
        hitPoint =  Q * (1.0f / PQk.w);
        return all(abs(hitPixel - ((1/camera_inv_resolution) * 0.5)) <= (1/camera_inv_resolution) * 0.5);
}

float4 PS(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
        int3 ss_load_coords = uint3(iPosition.xy, 0);
        float depth = txGBufferLinearDepth.Load(ss_load_coords).x;

        // Decode GBuffer information
        float  roughness;
        float3 wPos, N, albedo, specular_color, reflected_dir, view_dir;

        decodeGBuffer(iPosition.xy, wPos, N, albedo, specular_color, roughness, reflected_dir, view_dir);

        float4 view_normal = mul(float4(N.xyz, 0), camera_view);
        float4 view_pos = mul(float4(wPos, 1), camera_view);
        float3 reflected = normalize(reflect(view_pos, view_normal).xyz);

        // out parameters
        float2 hitPixel = float2(0.0f, 0.0f);
        float3 hitPoint = float3(0.0f, 0.0f, 0.0f);

        float jitter = CB_STRIDE > 1.0f ? float(int(iPosition.x + iPosition.y) & 1) * 0.5f : 0.0f;
        bool intersection = traceScreenSpaceRay(view_pos, reflected, jitter, hitPixel, hitPoint);
        hitPixel *= camera_inv_resolution;

        if(hitPixel.x > 1.0f || hitPixel.x < 0.0f || hitPixel.y > 1.0f || hitPixel.y < 0.0f)
        {
            intersection = false;
        }

        float4 color2 = txAlbedo.Sample(samClampLinear, iTex0);
        float4 color = txAlbedo.Sample(samClampLinear, hitPixel);
        //return float4(hitPixel, depth, 1) * (intersection ? 1.0f : 0.0f);

        return color;//float4(hitPixel, depth, 1);
}*/
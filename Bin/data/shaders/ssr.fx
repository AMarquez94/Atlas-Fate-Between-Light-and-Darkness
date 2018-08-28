//--------------------------------------------------------------------------------------
#include "pbr.fx"

static const int MAX_RAY_STEPS = 100;
static const int MAX_BIN_STEPS = 100;
static const float rayStep = 0.25f;

float4 BinarySearch(inout float3 reflected_ray, float3 hit_coord)
{
	float depth = 0;
	float depth_diff = 0;
	float4 projectedCoord;
	
	for (int i = 0; i < MAX_BIN_STEPS; i++)
	{
		projectedCoord = mul(float4(hit_coord, 1.f), camera_proj);
	  projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
    depth = txGBufferLinearDepth.SampleLevel(samClampPoint, projectedCoord.xy, 0).r;					
		depth_diff = hit_coord.z - depth;
		
		reflected_ray += 0.5;
		if(depth_diff > 0)
			hit_coord += reflected_ray;
		else
			hit_coord -= reflected_ray;
	}

	projectedCoord = mul(float4(hit_coord, 1.f), camera_proj);
	projectedCoord.xy /= projectedCoord.w;
	projectedCoord.xy = projectedCoord.xy * float2(0.5f, 0.5f) + float2(0.5f, 0.5f);
		
	return projectedCoord;
}

float4 RayMarching(float3 reflected_ray, float3 hit_coord)
{
	float depth = 0;
	float depth_diff = 0;
	float4 projectedCoord;
	reflected_ray *= rayStep;
	
	for (int i = 0; i < MAX_RAY_STEPS; i++)
	{
		hit_coord += reflected_ray;
		projectedCoord = mul(float4(hit_coord, 1.f), camera_proj);
	  projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
    depth = txGBufferLinearDepth.SampleLevel(samClampPoint, projectedCoord.xy, 0).r;					
		depth_diff =(hit_coord.z - depth);
		
		if(depth_diff <= 0.0){
			//return BinarySearch(reflected_ray, hit_coord);
			return float4(projectedCoord.xy, depth_diff, 1.0);	
		}
	}

  return float4(0, 0, 0, 0);
}

float4 PS(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	// Decode GBuffer information
	float  roughness;
	float3 wPos, N, albedo, specular_color, reflected_dir, view_dir;

  int3 ss_load_coords = uint3(iPosition.xy, 0);
	decodeGBuffer(iPosition.xy, wPos, N, albedo, specular_color, roughness, reflected_dir, view_dir);
	float reflectance = 1. - (roughness * roughness);
	float depth = txGBufferLinearDepth.Load(ss_load_coords).x;
	float4 N_rt = txGBufferNormals.Load(ss_load_coords);
	
	float3 ss_n = normalize(mul(N, camera_view));
	float3 ss_wpos = normalize(mul(wPos, camera_view));
	
	float4 ss_pos = float4(iTex0, depth, 1.f);
	float3 reflected_ray = normalize(reflect(ss_wpos, ss_n));
	//return float4(reflected_ray, 1);
	
	float4 coords = RayMarching(reflected_ray, ss_wpos.xyz);
	float4 color = txAlbedo.Sample(samClampLinear, iTex0); 
	float4 ssr = txAlbedo.SampleLevel(samClampLinear, reflected_ray.xy, 0); 
	
	float screenEdge = clamp( 1 - (coords.x + coords.y), 0.0, 1.0);
	return color;
}
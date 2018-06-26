#include "common.fx"

//--------------------------------------------------------------------------------------
// Light volume generation pass. Vertex
//--------------------------------------------------------------------------------------
void VS_IVLight(
    in float4 iPos     : POSITION
    , in float3 iNormal : NORMAL0
    , in float2 iTex0 : TEXCOORD0
    , in float2 iTex1 : TEXCOORD1
    , in float4 iTangent : NORMAL1
	, in TInstanceWorldData instance_data     // Stream 1
	
    , out float4 oPos : SV_POSITION
    , out float3 oNormal : NORMAL0
    , out float4 oTangent : NORMAL1
    , out float2 oTex0 : TEXCOORD0
    , out float2 oTex1 : TEXCOORD1
    , out float3 oWorldPos : TEXCOORD2
)
{
	float4x4 instance_world = getWorldOfInstance(instance_data);

	float4 world_pos = mul(iPos, instance_world);
	oPos = mul(world_pos, camera_view_proj);

	// Rotar la normal segun la transform del objeto
	oNormal = mul(iNormal, (float3x3)instance_world);
	oTangent.xyz = mul(iTangent.xyz, (float3x3)instance_world);
	oTangent.w = iTangent.w;

	// Las uv's se pasan directamente al ps
	oTex0 = iTex0;
	oTex1 = iTex1;
	oWorldPos = world_pos.xyz;
}

//--------------------------------------------------------------------------------------
// Light volume generation pass. Fragment
//--------------------------------------------------------------------------------------
float4 PS_IVLight( 
    in float4 iPos : SV_POSITION
    , float3 iNormal : NORMAL0
    , float4 iTangent : NORMAL1
    , float2 iTex0 : TEXCOORD0
    , float2 iTex1 : TEXCOORD1
    , float3 iWorldPos : TEXCOORD2
) : SV_Target
{
    // Sampling planes volumetric lights based shader.
    float shadow_factor = computeShadowFactor(iWorldPos);
    float camera_dist = length(iWorldPos - light_pos.xyz);
    float val = 1 / (1 + (camera_dist * camera_dist));

    // From wPos to Light
    float3 light_dir_full = light_pos.xyz - iWorldPos;
    float  distance_to_light = length(light_dir_full);
    float3 light_dir = light_dir_full / distance_to_light;
    float4 noise0 = txNoiseMap.Sample(samLinear, iTex0 * 1.0 + 0.002 * global_world_time * float2(.5, 0));
    float4 noise1 = txNoiseMap.Sample(samLinear, iTex0 * 2.0 + 0.081 * global_world_time * float2(.5, 0.1));
    float4 noise2 = txNoiseMap.Sample(samLinear, iTex0 * 4 + 0.008 * global_world_time * float2(.55, -0.123));

    float theta = dot(light_dir, -light_direction.xyz);
    float att_spot = clamp((theta - light_outer_cut) / 0.38, 0, 1);
    float clamp_spot = theta > light_angle ? att_spot : 0.0; // spot factor 
    float noise_clamp = 9 * (noise0.x * noise1.x * noise2.x);

    return float4(light_color.xyz, clamp_spot * val * noise_clamp) * shadow_factor * projectColor(iWorldPos);
}

#include "common.fx"

struct TInstanceLight {
  float4 InstanceWorld0 : TEXCOORD2;     // Stream 1
  float4 InstanceWorld1 : TEXCOORD3;    // Stream 1
  float4 InstanceWorld2 : TEXCOORD4;    // Stream 1
  float4 InstanceWorld3 : TEXCOORD5;    // Stream 1
};

//--------------------------------------------------------------------------------------
// Light volume generation pass. Vertex
//--------------------------------------------------------------------------------------
void VS_IVLight(
    in float4 iPos     : POSITION
    , in float3 iNormal : NORMAL0
    , in float2 iTex0 : TEXCOORD0
    , in float2 iTex1 : TEXCOORD1
    , in float4 iTangent : NORMAL1
  // From stream 1 we read the instance information 
		, in TInstanceWorldData instance_data     // Stream 1
		, in float4 InstanceLightPos   : TEXCOORD6   // TimeToLife, TimeBlendingOut, ...  
		, in float4 InstanceLightDir   : TEXCOORD7   // TimeToLife, TimeBlendingOut, ...  
		, in float4 InstanceLightValues   : TEXCOORD8   // TimeToLife, TimeBlendingOut, ...  
			
    , out float4 oPos : SV_POSITION
    , out float3 oNormal : NORMAL0
    , out float4 oTangent : NORMAL1
    , out float2 oTex0 : TEXCOORD0
    , out float2 oTex1 : TEXCOORD1
    , out float3 oWorldPos : TEXCOORD2
		, out float4 oLightPos : TEXCOORD3
		, out float4 oLightDir : TEXCOORD4
		, out float4 oLighValues : TEXCOORD5
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
	oLightPos = InstanceLightPos;
	oLightDir = InstanceLightDir;
	oLighValues = InstanceLightValues;
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
		, float4 iLightPos : TEXCOORD3
		, float4 iLightDir : TEXCOORD4
		, float4 iLightValues : TEXCOORD5
) : SV_Target
{
    // Sampling planes volumetric lights based shader.
    float shadow_factor = computeShadowFactor(iWorldPos);
    float camera_dist = length(iWorldPos - iLightPos.xyz);
    float val = 1 / (1 + (camera_dist * camera_dist));

    // From wPos to Light
    float3 light_dir_full = iLightPos.xyz - iWorldPos;
    float  distance_to_light = length(light_dir_full);
    float3 light_dir = light_dir_full / distance_to_light;
    float4 noise0 = txNoiseMap.Sample(samLinear, iTex0 * 1.0 + 0.002 * global_world_time * float2(.5, 0));
    //float4 noise1 = txNoiseMap.Sample(samLinear, iTex0 * 2.0 + 0.081 * global_world_time * float2(.5, 0.1));
    //float4 noise2 = txNoiseMap.Sample(samLinear, iTex0 * 4 + 0.008 * global_world_time * float2(.55, -0.123));

    float theta = dot(light_dir, -iLightDir.xyz);
    float att_spot = clamp((theta - iLightValues.z) / 0.78, 0, 1);
    float clamp_spot = theta > iLightValues.x ? att_spot : 0.0; // spot factor 
    //float noise_clamp = 9 * (noise0.x * noise1.x * noise2.x);
		
    return float4(1,1,1, clamp_spot * val * noise0.x);
}

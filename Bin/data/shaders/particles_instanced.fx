#include "common.fx"

// ----------------------------------------
void VS_Particles(
	  in float4 iPos   : POSITION
 	, in TInstanceWorldData instance_data
	, in float2 iMinUv   : TEXCOORD6   // 
	, in float2 iMaxUv   : TEXCOORD7  // 
	, in float4 iColorP   : TEXCOORD8   // 
	
	, out float4 oPos : SV_POSITION
	, out float2 oTex0 : TEXCOORD0
	, out float2 oMinUv : TEXCOORD1
  , out float2 oMaxUv : TEXCOORD2
  , out float4 oColorP : TEXCOORD3
  )
{

	float4x4 instance_world = getWorldOfInstance(instance_data);
	
  float4 pos = iPos - float4(0.5, 0.5, 0, 0);
  float4 world_pos = mul(pos, instance_world);
  oPos = mul(world_pos, camera_view_proj);
  oTex0  = iPos.xy;
	
	oMinUv = iMinUv;
	oMaxUv = iMaxUv;
	oColorP = iColorP;
}

// ----------------------------------------
void VS_GBuffer_Particles(
	  in float4 iPos   : POSITION
 	, in TInstanceWorldData instance_data
	, in float2 iMinUv   : TEXCOORD6   // 
	, in float2 iMaxUv   : TEXCOORD7  // 
	, in float4 iColorP   : TEXCOORD8   // 
	
	, out float4 oPos : SV_POSITION
	, out float2 oTex0 : TEXCOORD0
	, out float2 oMinUv : TEXCOORD1
  , out float2 oMaxUv : TEXCOORD2
  , out float4 oColorP : TEXCOORD3
	, out float3 oWorldPos : TEXCOORD4
  )
{

	float4x4 instance_world = getWorldOfInstance(instance_data);
	
  float4 pos = iPos - float4(0.5, 0.5, 0, 0);
  float4 world_pos = mul(pos, instance_world);
	oWorldPos = world_pos.xyz;
  oPos = mul(world_pos, camera_view_proj);
  oTex0  = iPos.xy;
	
	oMinUv = iMinUv;
	oMaxUv = iMaxUv;
	oColorP = iColorP;
}

// ----------------------------------------
// 							PS PARTICLES
// ----------------------------------------

float4 PS_Particles(
	  in float4 iPos : SV_POSITION
	, in float2 iTex0 : TEXCOORD0
	, in float2 iMinUv : TEXCOORD1
  , in float2 iMaxUv : TEXCOORD2
  , in float4 iColorP : TEXCOORD3
  ) : SV_Target
{
  float2 finalUV = lerp(iMinUv, iMaxUv, iTex0);
  float4 oDiffuse = txAlbedo1.Sample(samLinear, finalUV);
	return float4(1,1,1,1);
  float4 finalColor = float4(oDiffuse.rgb * iColorP.rgb, oDiffuse.a * iColorP.a);
  return finalColor;
}


// ----------------------------------------
void PS_GBuffer_Particles(
	  in float4 iPos : SV_POSITION
	, in float2 iTex0 : TEXCOORD0
	, in float2 iMinUv : TEXCOORD1
  , in float2 iMaxUv : TEXCOORD2
  , in float4 iColorP : TEXCOORD3
	, in float4 iWorldPos : TEXCOORD4
	, out float4 o_albedo : SV_Target0
	, out float4 o_normal : SV_Target1
  )
{
  float2 finalUV = lerp(iMinUv, iMaxUv, iTex0);
  float4 oDiffuse = txAlbedo1.Sample(samLinear, finalUV);
  float4 finalColor = float4(oDiffuse.rgb * iColorP.rgb, 1);
	o_albedo = finalColor;
	o_normal = float4(1,1,1,1);
	//o_albedo.a = txMetallic.Sample(samLinear, iTex0).r;
	
	/*o_selfIllum =  txEmissive.Sample(samLinear, iTex0) * self_intensity;
	o_selfIllum.xyz *= self_color.xyz;
	o_selfIllum.a = txAOcclusion.Sample(samLinear, iTex0).r;
	
	// Save roughness in the alpha coord of the N render target
	float roughness = txRoughness.Sample(samLinear, iTex0).r;
	float3 N = computeNormalMap(iNormal, iTangent, iTex0);
	o_normal = encodeNormal(N, roughness);
	
	// Compute the Z in linear space, and normalize it in the range 0...1
	// In the range z=0 to z=zFar of the camera (not zNear)
	float3 camera2wpos = iWorldPos - camera_pos;
	o_depth = dot(camera_front.xyz, camera2wpos) / camera_zfar;*/
}
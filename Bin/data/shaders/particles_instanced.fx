#include "common.fx"

// ----------------------------------------
void VS_Particles(
	in float4 iPos     : POSITION
	, in float3 iNormal : NORMAL0
	, in float2 iTex0 : TEXCOORD0
	, in float2 iTex1 : TEXCOORD1
	, in float4 iTangent : NORMAL1
 	, in TInstanceWorldData instance_data
	, in float2 iMinUv   : TEXCOORD6   // 
	, in float2 iMaxUv   : TEXCOORD7  // 
	, in float4 iColorP   : TEXCOORD8   // 
	
	, out float4 oPos : SV_POSITION
	, out float3 oNormal : NORMAL0
	, out float4 oTangent : NORMAL1
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
	
	// Rotar la normal segun la transform del objeto
	oNormal = mul(iNormal, (float3x3)obj_world);
	oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
	oTangent.w = iTangent.w;
	
	oMinUv = iMinUv;
	oMaxUv = iMaxUv;
	oColorP = iColorP;
}

// ----------------------------------------
void PS_Particles(
	  in float4 iPos : SV_POSITION
	, in float3 iNormal : NORMAL0
	, in float4 iTangent : NORMAL1
	, in float2 iTex0 : TEXCOORD0
	, in float2 iMinUv : TEXCOORD1
  , in float2 iMaxUv : TEXCOORD2
  , in float4 iColorP : TEXCOORD3
	, in float3 iWorldPos : TEXCOORD4
	, out float4 o_albedo : SV_Target0
	, out float4 o_normal : SV_Target1
	, out float1 o_depth : SV_Target2
	, out float4 o_selfIllum : SV_Target3
  )
{
  float2 finalUV = lerp(iMinUv, iMaxUv, iTex0);
  float4 oDiffuse = txAlbedo1.Sample(samLinear, finalUV);
	
	o_albedo = float4(iColorP.rgb, oDiffuse.a * iColorP.a);

	/*o_selfIllum =  txEmissive.Sample(samLinear, iTex0) * self_intensity;
	o_selfIllum.xyz *= self_color.xyz;
	o_selfIllum.a = txAOcclusion.Sample(samLinear, iTex0).r;
	*/
	
	float3 camera2wpos = iWorldPos - camera_pos;
	o_depth = dot(camera_front.xyz, camera2wpos) / camera_zfar;

}
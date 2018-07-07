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
  float4 finalColor = float4(oDiffuse.rgb * iColorP.rgb, oDiffuse.a * iColorP.a);
  return finalColor;
}
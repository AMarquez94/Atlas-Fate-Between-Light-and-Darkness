#include "common.fx"

void VS(
  in float4 iPos     : POSITION
, in float3 iNormal  : NORMAL0
, in float2 iTex0    : TEXCOORD0
, in float2 iTex1    : TEXCOORD1
, in float4 iTangent : NORMAL1

, out float4 oPos      : SV_POSITION
, out float3 oNormal   : NORMAL0
, out float4 oTangent  : NORMAL1
, out float2 oTex0     : TEXCOORD0
, out float2 oTex1     : TEXCOORD1
, out float3 oWorldPos : TEXCOORD2

)
{

  float4 world_pos = mul(iPos, obj_world);
  oPos = mul(world_pos, camera_view_proj);

  // Rotar la normal segun la transform del objeto
  oNormal = mul(iNormal, (float3x3)obj_world);
  oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
  oTangent.w = iTangent.w;

  // Las uv's se pasan directamente al ps
  oTex0 = iTex0;
  oTex1 = iTex1;
  oWorldPos = world_pos.xyz;
}

float4 PS_Moon(
	float4 Pos : SV_POSITION
	, float3 iNormal : NORMAL0
	, float4 iTangent : NORMAL1
	, float2 iTex0 : TEXCOORD0
	, float2 iTex1 : TEXCOORD1
	, float3 iWorldPos : TEXCOORD2
) : SV_Target
{
	float3 viewdir = normalize(iWorldPos - camera_pos.xyz);
  float4 color = float4(1,1,1,1);
	color.a = pow(dot(viewdir, iNormal),5);
	//color.a *=dot(iNormal, float3(3.5, 71, -234));
	//color.a = saturate(color.a);
	
	color.a = pow(color.a, 5);
	color.a *= 0.1;
	return color;
}

float4 PS_Moon_Atmosphere(
	float4 Pos : SV_POSITION
	, float3 iNormal : NORMAL0
	, float4 iTangent : NORMAL1
	, float2 iTex0 : TEXCOORD0
	, float2 iTex1 : TEXCOORD1
	, float3 iWorldPos : TEXCOORD2
) : SV_Target
{
iTex0+= global_world_time * 0.01;
  return txAlbedo.Sample(samLinear, iTex0) * 0.55;
}
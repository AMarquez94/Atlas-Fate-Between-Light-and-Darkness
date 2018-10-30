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

void PS_Moon(
	float4 Pos       : SV_POSITION
	, float3 iNormal : NORMAL0
	, float4 iTangent : NORMAL1
	, float2 iTex0 : TEXCOORD0
	, float2 iTex1 : TEXCOORD1
	, float3 iWorldPos : TEXCOORD2
	, out float4 o_albedo : SV_Target0
	, out float4 o_normal : SV_Target1
	, out float1 o_depth : SV_Target2
	, out float4 o_selfIllum : SV_Target3
)
{
	o_albedo = txAlbedo.Sample(samLinear, iTex0);
	o_albedo.a = 0;
	o_selfIllum =  txEmissive.Sample(samLinear, iTex0);
	o_selfIllum.xyz *= self_color.xyz * self_intensity * 15;
	o_selfIllum.a = txAOcclusion.Sample(samLinear, iTex0).r;
	
	float3 dir_to_eye = normalize(camera_pos.xyz - iWorldPos.xyz);
	float3 dir_to_light = normalize( ( camera_pos + global_delta_position) - iWorldPos.xyz);
	
	// Save roughness in the alpha coord of the N render target
	float roughness = txRoughness.Sample(samLinear, iTex0).r;
	float3 N = computeNormalMap(iNormal, iTangent, iTex0);
	o_normal = encodeNormal(N, roughness);
	
	// Compute the Z in linear space, and normalize it in the range 0...1
	// In the range z=0 to z=zFar of the camera (not zNear)
	float3 camera2wpos = iWorldPos - camera_pos;
	o_depth = dot(camera_front.xyz, camera2wpos) / camera_zfar;
}

void PS_Planet(
	float4 Pos       : SV_POSITION
	, float3 iNormal : NORMAL0
	, float4 iTangent : NORMAL1
	, float2 iTex0 : TEXCOORD0
	, float2 iTex1 : TEXCOORD1
	, float3 iWorldPos : TEXCOORD2
	, out float4 o_albedo : SV_Target0
	, out float4 o_normal : SV_Target1
	, out float1 o_depth : SV_Target2
	, out float4 o_selfIllum : SV_Target3
)
{
	o_albedo = txAlbedo.Sample(samLinear, iTex0);
	o_albedo.a = 0;
	o_selfIllum =  txEmissive.Sample(samLinear, iTex0);
	o_selfIllum.xyz *= self_color.xyz * self_intensity * 5;
	o_selfIllum.a = txAOcclusion.Sample(samLinear, iTex0).r;
	
	float3 dir_to_eye = normalize(camera_pos.xyz - iWorldPos.xyz);
	float3 dir_to_light = normalize( ( camera_pos + global_delta_position) - iWorldPos.xyz);
	float falloff = dot(normalize(iNormal), dir_to_light);
	o_albedo.xyz *= pow(1 - abs(falloff), 2);
	o_selfIllum.xyz *= pow(1 - abs(falloff), 1);
	
	// Save roughness in the alpha coord of the N render target
	float roughness = txRoughness.Sample(samLinear, iTex0).r;
	float3 N = computeNormalMap(iNormal, iTangent, iTex0);
	o_normal = encodeNormal(N, roughness);
	
	// Compute the Z in linear space, and normalize it in the range 0...1
	// In the range z=0 to z=zFar of the camera (not zNear)
	float3 camera2wpos = iWorldPos - camera_pos;
	o_depth = dot(camera_front.xyz, camera2wpos) / camera_zfar;
}

float4 PS_Moon_Light(
	float4 Pos : SV_POSITION
	, float3 iNormal : NORMAL0
	, float4 iTangent : NORMAL1
	, float2 iTex0 : TEXCOORD0
	, float2 iTex1 : TEXCOORD1
	, float3 iWorldPos : TEXCOORD2
) : SV_Target
{
	float3 viewdir = normalize(iWorldPos - camera_pos.xyz);
  float4 color = obj_color;
	
	// Fresnel component
	float3 dir_to_eye = normalize(camera_pos.xyz - iWorldPos.xyz);
	float3 dir_to_light = normalize( ( camera_pos + global_delta_position) - iWorldPos.xyz);
	float3 N = normalize(iNormal.xyz);
	float fresnel = dot(N, -dir_to_eye);
	float falloff = clamp(dot(N, dir_to_light), 0, 1);
		
	color.a *= pow(abs(fresnel),4) * 2;
	//color.a *= falloff - ( (1- self_intensity) * falloff) + (1- self_intensity);
		
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
	iTex0+= global_world_time * 0.0035;
	float3 dir_to_eye = normalize(camera_pos.xyz - iWorldPos.xyz);
	float3 dir_to_light = normalize( ( camera_pos + global_delta_position) - iWorldPos.xyz);
	float fresnel = dot(iNormal, -dir_to_eye);
	float flow = 1 - abs(dir_to_light);
  return txAlbedo.Sample(samLinear, iTex0) * flow * self_intensity * obj_color;
}
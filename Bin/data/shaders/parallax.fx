//--------------------------------------------------------------------------------------
#include "common.fx"

float2 ComputeParallax(float2 texCoords, float3 view_dir) {

  const float minLayers = 15.0;
  const float maxLayers = 30.0;
  float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), view_dir)));
  float layerDepth = 1.0 / numLayers;
  float currentLayerDepth = 0.0;
  float2 P = view_dir.xy * 0.075;
  float2 deltaTexCoords = P / numLayers;

  float2 currentTexCoords = texCoords;
  float currentDepthMapValue = 1 - txHeight.Sample(samLinear, currentTexCoords).r;

  [unroll(330)]
  while (currentLayerDepth < currentDepthMapValue)
  {
    currentTexCoords -= deltaTexCoords;
    currentDepthMapValue = 1 - txHeight.Sample(samLinear, currentTexCoords).r;
    currentLayerDepth += layerDepth;
  }

  float2 prevTexCoords = currentTexCoords + deltaTexCoords;
  float afterDepth = currentDepthMapValue - currentLayerDepth;
  float beforeDepth = (1 - txHeight.Sample(samLinear, prevTexCoords).r) - currentLayerDepth + layerDepth;
  float weight = afterDepth / (afterDepth - beforeDepth);
  float2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

  return finalTexCoords;
}

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Vertex
//--------------------------------------------------------------------------------------
void VS_GBuffer(
	in float4 iPos     : POSITION
	, in float3 iNormal : NORMAL0
	, in float2 iTex0 : TEXCOORD0
	, in float2 iTex1 : TEXCOORD1
	, in float4 iTangent : NORMAL1

	, out float4 oPos : SV_POSITION
	, out float3 oNormal : NORMAL0
	, out float4 oTangent : NORMAL1
	, out float2 oTex0 : TEXCOORD0
	, out float2 oTex1 : TEXCOORD1
	, out float3 oWorldPos : TEXCOORD2
	, out float3 oTanView : TEXCOORD3
	, out float3 oTanFrag : TEXCOORD4
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

	float3x3 TBN = transpose(computeTBN(oNormal, oTangent));
	oTanView = mul(camera_pos, TBN);
	oTanFrag = mul(oWorldPos, TBN);
}

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Pixel
//--------------------------------------------------------------------------------------
void PS_GBuffer_Parallax(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
  , float3 iTanView : TEXCOORD3
  , float3 iTanFrag : TEXCOORD4
	
  , out float4 o_albedo : SV_Target0
  , out float4 o_normal : SV_Target1
  , out float1 o_depth : SV_Target2
  , out float4 o_selfIllum : SV_Target3
)
{
	// Shift the uv's by the parallax effect
	float3 view_dir = normalize(iTanView - iTanFrag);
	iTex0 = ComputeParallax(iTex0, view_dir);
	
	o_albedo = txAlbedo.Sample(samLinear, iTex0);
	o_albedo.a = txMetallic.Sample(samLinear, iTex0).r;
	o_selfIllum = txEmissive.Sample(samLinear, iTex0) * self_intensity;
	o_selfIllum.xyz *= self_color;

	float roughness = txRoughness.Sample(samLinear, iTex0).r;
	float3 N = computeNormalMap(iNormal, iTangent, iTex0);
	o_normal = encodeNormal(N, roughness);

	if (scalar_metallic >= 0.f)
	o_albedo.a = scalar_metallic;
	if (scalar_roughness >= 0.f)
	o_normal.a = scalar_roughness;

	float3 camera2wpos = iWorldPos - camera_pos;
	o_depth = dot(camera_front.xyz, camera2wpos) / camera_zfar;
}

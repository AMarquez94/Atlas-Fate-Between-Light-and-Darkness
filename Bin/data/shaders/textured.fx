//--------------------------------------------------------------------------------------
#include "common.fx"

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
  float4 Pos : SV_POSITION;
  float3 N : NORMAL;
  float2 UV : UV;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(
  float4 Pos : POSITION
, float3 N   : NORMAL
, float2 UV  : TEXCOORD0
)

{
  VS_OUTPUT output = (VS_OUTPUT)0;
  output.Pos = mul(Pos, obj_world);
  output.Pos = mul(output.Pos, camera_view);
  output.Pos = mul(output.Pos, camera_proj);
	output.Pos = output.Pos.xyww;
	
  // Rotate the normal
  output.N = mul(N, (float3x3)obj_world);
  output.UV = UV;
  return output;
}

VS_OUTPUT VS_Skin(
  float4 iPos : POSITION
, float3 iN   : NORMAL
, float2 iUV  : TEXCOORD0
, int4   iBones   : BONES
, float4 iWeights : WEIGHTS
)
{

  float4x4 skin_mtx = getSkinMtx( iBones, iWeights );
  float4 skinned_Pos = mul(iPos, skin_mtx);

  VS_OUTPUT output = (VS_OUTPUT)0;
  // The world matrix is inside the bones matrixs
  output.Pos = mul(skinned_Pos, camera_view);
  output.Pos = mul(output.Pos, camera_proj);
  // Rotate the normal
  output.N = mul(iN, (float3x3)obj_world);
  output.UV = iUV;
  return output;
}

float4 PS(
	float4 Pos : SV_POSITION
	, float3 iNormal : NORMAL0
	, float4 iTangent : NORMAL1
	, float2 iTex0 : TEXCOORD0
	, float2 iTex1 : TEXCOORD1
	, float3 iWorldPos : TEXCOORD2
) : SV_Target
{
	//return float4(1,1,1,1);
  return txAlbedo.Sample(samLinear, input.UV);
}

void VS_SKIN_GBuffer(
	float4 iPos : POSITION
	, float3 iN : NORMAL
	, float2 iUV : TEXCOORD
	, float4 iTangent : TANGENT
	, int4   iBones : BONES
	, float4 iWeights : WEIGHTS

	, out float4 oPos : SV_POSITION
	, out float3 oNormal : NORMAL0
	, out float4 oTangent : NORMAL1
	, out float2 oTex0 : TEXCOORD0
	, out float2 oTex1 : TEXCOORD1
	, out float3 oWorldPos : TEXCOORD2
)
{
	// Faking the verterx shader by now since we don't have tangents...
	float4x4 skin_mtx = getSkinMtx(iBones, iWeights);
	float4 skinned_Pos = mul(float4(iPos.xyz * BonesScale, 1), skin_mtx);

	oPos = mul(skinned_Pos, camera_view_proj); // Transform to viewproj, w_m inside skin_m
	oNormal = mul(iN, (float3x3)obj_world); // Rotate the normal
	oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
	oTangent.w = iTangent.w;

	oTex0 = iUV;
	oTex1 = iUV;
	oWorldPos = skinned_Pos.xyz;
}
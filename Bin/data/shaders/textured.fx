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

float4 PS(VS_OUTPUT input) : SV_Target
{
  return txAlbedo.Sample(samLinear, input.UV);
}
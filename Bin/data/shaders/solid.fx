//--------------------------------------------------------------------------------------
#include "common.fx"

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
  float4 Pos : SV_POSITION;
  float4 Color : COLOR0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(float4 Pos : POSITION, float4 Color : COLOR)
{
  VS_OUTPUT output = (VS_OUTPUT)0;
  output.Pos = mul(Pos, obj_world);
  output.Pos = mul(output.Pos, camera_view);
  output.Pos = mul(output.Pos, camera_proj);
  output.Color = Color * obj_color;
  return output;
}

VS_OUTPUT VS_Objs(
	 float4 Pos : POSITION
 , float3 N : NORMAL
 , float2 UV : TEXCOORD0
 )
{
  VS_OUTPUT output = (VS_OUTPUT)0;
  output.Pos = mul(Pos, obj_world);
  output.Pos = mul(output.Pos, camera_view);
  output.Pos = mul(output.Pos, camera_proj);
  output.Color = obj_color;
  return output;
}

VS_OUTPUT VS_Objs_uv2(
	 float4 Pos : POSITION
 , float3 N : NORMAL
 , float2 UV : TEXCOORD0
 , float2 UV2 : TEXCOORD1
 )
{
  VS_OUTPUT output = (VS_OUTPUT)0;
  output.Pos = mul(Pos, obj_world);
  output.Pos = mul(output.Pos, camera_view);
  output.Pos = mul(output.Pos, camera_proj);
  output.Color = obj_color;
  return output;
}

VS_OUTPUT VS_Objs_Skin(
   float4 iPos : POSITION
 , float3 iN : NORMAL
 , float2 iUV : TEXCOORD0
 , int4   iBones   : BONES
 , float4 iWeights : WEIGHTS
 )
{

  // This matrix will be reused for the position, Normal, Tangent, etc
  float4x4 skin_mtx = getSkinMtx( iBones, iWeights );
  float4 skinned_Pos = mul(iPos, skin_mtx);

  VS_OUTPUT output = (VS_OUTPUT)0;
  // The world matrix is inside the bones matrixs
  output.Pos = mul(skinned_Pos, camera_view);
  output.Pos = mul(output.Pos, camera_proj);
  output.Color = obj_color;
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
  return input.Color;
}
#include "common.fx"

struct VS_OUTPUT
{
  float4 Pos   : SV_POSITION;
  // To generate the shadows I don't need the UV's or the normal
};

// ----------------------------------------------
VS_OUTPUT VS(
  float4 iPos : POSITION,
  float3 iN   : NORMAL,
  float2 iUV  : TEXCOORD0
  )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    float4 world_pos = mul( iPos, obj_world );
    output.Pos = mul(world_pos, camera_view );
    output.Pos = mul(output.Pos, camera_proj );
    return output;
}

// -----------------------------------------------------
VS_OUTPUT VS_Skin(
    float4 iPos : POSITION
  , float3 iN   : NORMAL
  , float2 iUV  : TEXCOORD0
  , int4   iBones   : BONES
  , float4 iWeights : WEIGHTS
  )
{

  float4x4 skin_mtx = getSkinMtx( iBones, iWeights );

  // Skinned pos
  float4 world_pos = mul(iPos, skin_mtx);
  VS_OUTPUT output = (VS_OUTPUT)0;
  output.Pos = mul(world_pos, camera_view );
  output.Pos = mul(output.Pos, camera_proj );
  return output;
}

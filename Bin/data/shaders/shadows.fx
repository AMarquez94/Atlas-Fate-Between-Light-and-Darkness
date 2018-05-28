#include "common.fx"

struct VS_OUTPUT
{
  float4 Pos   : SV_POSITION;
  // To generate the shadows I don't need the UV's or the normal
};

// ----------------------------------------------
VS_OUTPUT VS(
  float4 iPos : POSITION
  )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    float4 world_pos = mul( iPos, obj_world );
    output.Pos = mul(world_pos, camera_view_proj);
    return output;
}

// ----------------------------------------------
VS_OUTPUT VS_Instanced(
    in float4 iPos : POSITION
  , in TInstanceWorldData instance_data     // Stream 1
)
{
  float4x4 instance_world = getWorldOfInstance(instance_data);

  VS_OUTPUT output = (VS_OUTPUT)0;
  float4 world_pos = mul(iPos, instance_world);
  output.Pos = mul(world_pos, camera_view_proj);
  return output;
}

// -----------------------------------------------------
VS_OUTPUT VS_Skin(
    float4 iPos : POSITION
  , float3 iN   : NORMAL
  , float2 iUV  : TEXCOORD0
	, float4 iTangent : TANGENT
  , int4   iBones   : BONES
  , float4 iWeights : WEIGHTS
  )
{

  float4x4 skin_mtx = getSkinMtx( iBones, iWeights );

  // Skinned pos
  float4 world_pos = mul(iPos, skin_mtx);
  VS_OUTPUT output = (VS_OUTPUT)0;
  output.Pos = mul(world_pos, camera_view_proj);
  return output;
}

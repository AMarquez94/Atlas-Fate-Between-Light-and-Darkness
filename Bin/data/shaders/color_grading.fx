#include "common.fx"

//--------------------------------------------------------------------------------------
// This shader is expected to be used only with the mesh unitQuadXY.mesh
// Where the iPos goes from 0,0..1,1
void VS(
    in float4 iPos : POSITION
  , out float4 oPos : SV_POSITION
  , out float2 oTex0 : TEXCOORD0
)
{
  // Passthrough of coords and UV's
  oPos = float4(iPos.x * 2 - 1., 1 - iPos.y * 2, 0, 1);
  oTex0 = iPos.xy;
}

//--------------------------------------------------------------------------------------
float4 PS(
    in float4 iPosition : SV_Position
  , in float2 iTex0 : TEXCOORD0
) : SV_Target
{
  float4 in_color = txAlbedo.Sample(samClampLinear, iTex0.xy);
  // Use the input color as coords in the range 0..1 to do a lut (LookUpTable)
  float4 out_color = txLUT.Sample(samClampLinear, in_color.xyz);
  return lerp( in_color, out_color, global_shared_fx_amount );
}

#include "common.fx"

//--------------------------------------------------------------------------------------
// This shader is expected to be used only with the mesh unitQuadXY.mesh
// Where the iPos goes from 0,0..1,1
void VS(
    in float4 iPos : POSITION
  , in float2 iColor : COLOR0
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

  float2 v2c = iTex0 - blur_center;
  float2 distance2center = length( v2c );
  v2c = normalize( v2c );

  float amount = smoothstep( 0, blur_d.y, distance2center.x );      // 0.. comp.radius
  amount *= blur_d.x;                                             // comp.amount 
  //return amount;

  v2c *= amount * camera_inv_resolution.xx * distance2center * 10;  // x10 to amplify effect
  
  // N taps moving to the center
  float4 cfinal = float4(0,0,0,0);

  for( int i=0; i<16; ++i ) {
    float4 ctap  = txAlbedo.Sample(samClampLinear, iTex0 - v2c * i);
    cfinal += ctap;
  }

  return cfinal / 16;
}

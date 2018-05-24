//--------------------------------------------------------------------------------------
#include "common.fx"

// https://stackoverflow.com/questions/34601325/directx11-read-stencil-bit-from-compute-shader
Texture2D<uint2>    txBackBufferStencil        SLOT( TS_ALBEDO );


// In case we want to use the stencil test, we must have the zbuffer+stencil from the backbuffer
// actived when rendering the outlines pass
// But we can't read the current values as it's 
// In the tech 'outlines' add this ps entry point instead of the default PS
//    "ps_entry_point": "PS_Solid",
float4 PS_Solid() : SV_Target
{
  return float4(0,0.5,0,0.5);
}

//--------------------------------------------------------------------------------------
float4 PS(
  float4 iPosition : SV_POSITION
, float2 UV        : TEXCOORD0
  ) : SV_Target
{
  int3 ss_load_coords = uint3(iPosition.xy, 0);

  // Can't use sample because it's a texture of ints
  // stencil value is in the green channel
  uint s_cc = txBackBufferStencil.Load(ss_load_coords).g;

  float a = global_shared_fx_amount;

  // In case we want just the pixels inside
  //if( s_cc != 0 ) return float4( 1,1,0,0.5 * a); 

  // n = north, s = south, e = east, w = west, c = center
  uint s_nw = txBackBufferStencil.Load(ss_load_coords + int3(1,-1,0)).y;
  uint s_nc = txBackBufferStencil.Load(ss_load_coords + int3(0,-1,0)).y;
  uint s_ne = txBackBufferStencil.Load(ss_load_coords + int3(-1,-1,0)).y;
  uint s_cw = txBackBufferStencil.Load(ss_load_coords + int3( 1,0,0)).y;
  //   s_cc
  uint s_ce = txBackBufferStencil.Load(ss_load_coords + int3(-1,0,0)).y;
  uint s_sw = txBackBufferStencil.Load(ss_load_coords + int3(1, 1,0)).y;
  uint s_sc = txBackBufferStencil.Load(ss_load_coords + int3(0, 1,0)).y;
  uint s_se = txBackBufferStencil.Load(ss_load_coords + int3(-1, 1,0)).y;

  // We expect all the 3x3 pixels to have the same value 
  int sum_stencils = s_nw + s_nc + s_ne + s_cw + s_cc + s_ce + s_sw + s_sc + s_se;
  uint diff = sum_stencils - s_cc * 9;
  // If not we are in the border
  if( diff != 0 )
    return float4( 1,0,0,a); 

  // else, or we are inside ( stencil != 0 ) 
  if( s_cc != 0 ) {
    // Use different stencil values for different colors!
    return float4( 0.5,0,0,0.5*a); 
  }

  // or we are outside, all zeros.
  return float4( 0,0,0,0 );
}
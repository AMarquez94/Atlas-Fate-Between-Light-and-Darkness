//--------------------------------------------------------------------------------------
#include "common.fx"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
void VS(
  // From Stream 0 from unit_quad_pos_xy.mesh
  in float4 iPos : POSITION

  // From Stream 1 from particles_instanced.mesh
, in float4 iCenter : TEXCOORD1   // Angle comes in the .w of the iCenter
, in float4 iColor : TEXCOORD2
, in float4 iXtras : TEXCOORD3    // scale.xy, nframe

, out float4 oPos : SV_POSITION
, out float2 oUV : TEXCOORD0
, out float4 oColor : TEXCOORD1

)
{
  float angle = 0; //iCenter.w;
  float2 scale = iXtras.xy;

  // Compute uv's in base of the frame number
  float nframe = iXtras.z + global_world_time * 20;
  int   nframes_per_axis = 4;
  int   iframe = (int)nframe;
  int   ifx = iframe % 4;
  int   ify = (int) (iframe / 4);
  float2 uv = float2(ifx, ify) / nframes_per_axis;
  // Add the local coord to get the uv's for each vertex of the quad
  uv += iPos * ( 1.0 / nframes_per_axis);

  float3 local_pos = iPos.xyz * 2. - 1.;
  local_pos.xy *= scale.xy;

  // Rotate the particle
  float cs = cos( angle );
  float ss = sin( angle );

  float4 world_pos = float4(
      iCenter.xyz 
    + cs * ( local_pos.x * camera_left + local_pos.y * camera_up )
    + ss * ( local_pos.x * camera_up - local_pos.y * camera_left )
    , 1);
  oPos = mul(world_pos, camera_view_proj);
  oUV = 1-uv;

  // Remove the color for the fire sample
  oColor = float4( 1,1,1,1); //float4( iColor.xyz, 1 );
}

//--------------------------------------------------------------------------------------
float4 PS(
    float4 iPos : SV_POSITION
  , float2 iUV : TEXCOORD0
  , float4 iColor : TEXCOORD1
) : SV_Target
{
  float4 texture_color = txAlbedo.Sample(samLinear, iUV) * iColor;
  return texture_color;
}
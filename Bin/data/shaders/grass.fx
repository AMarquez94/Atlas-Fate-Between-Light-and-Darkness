//--------------------------------------------------------------------------------------
#include "common.fx"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
void VS(
  // From Stream 0 from unit_quad_pos_xy.mesh
  in float4 iPos : POSITION0
, in uint   iInstancedID : SV_InstanceID

  // From Stream 1 from grass_instanced.mesh
, in float4 iCenter : POSITION1

, out float4 oPos : SV_POSITION
, out float2 oUV : TEXCOORD0
, out float4 oColor : TEXCOORD1
, out float3 oWorldPos : TEXCOORD2

)
{
  // iPos goes from 0...1
  // local_pos goes from -1 ... 1
  float3 local_pos = iPos.xyz * 2. - 1.;
  local_pos.y += 1.0;

  // At this point local_pos goes from 0..2
  float unit_rand_val = ( 1 + sin( iInstancedID ) ) * 0.5f;

  // To fade out the grass as it goes far from the camera render position
  float z_start_fadeout = 150; 
  float alpha = 1.0 - smoothstep( z_start_fadeout, z_start_fadeout + 50, length ( iCenter.xyz - camera_pos.xyz ) );

  float scale_factor = 2.0f + 2.0f * unit_rand_val;

  local_pos *= scale_factor * alpha;

  float2 uv = iPos.xy;

  // Add some wind effect
  local_pos.x += sin( ( 3 + 1 * unit_rand_val ) * global_world_time ) * local_pos.y * 0.025;

  // Base position is the position of the instance
  float4 world_pos = iCenter;
  // Option1 : Align to the camera
  // world_pos.xyz += ( local_pos.x * camera_left + local_pos.y * camera_up );
  // Option2 : Add a random orientation around the vertical axis
  world_pos.xyz += 2 * ( local_pos.x * float3(cos(3*iInstancedID),0, sin(3*iInstancedID)) + local_pos.y * float3(0,1,0));

  oPos = mul(world_pos, camera_view_proj);
  oUV = 1-uv;

  // Remove the color for the fire sample
  oColor = float4( 1,unit_rand_val,1,alpha); //float4( iColor.xyz, 1 );

  // To compute the shadows in the PS
  oWorldPos = world_pos.xyz;
}

//--------------------------------------------------------------------------------------
void PS(
    float4 iPos : SV_POSITION
  , float2 iUV : TEXCOORD0
  , float4 iColor : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2

  , out float4 o_albedo : SV_Target0
  , out float4 o_normal : SV_Target1
  , out float1 o_depth  : SV_Target2

)
{
  float4 texture_color = txAlbedo.Sample(samLinear, iUV) * iColor;

  if ( texture_color.a < 0.3 ) 
    discard;

  o_albedo.xyz = texture_color.xyz;
  o_albedo.a = 0.;

  //float shadow = computeShadowFactor( iWorldPos );
  //texture_color.xyz *= shadow;
  //texture_color.a = 1;

  // Fake values for metallic, roughness and N
  float3 N = float3(0,1,0);

  // Save roughness in the alpha coord of the N render target
  float roughness = 1.; //txRoughness.Sample(samLinear, iTex0).r;
  o_normal = encodeNormal( N, roughness );

  // Compute the Z in linear space, and normalize it in the range 0...1
  float3 camera2wpos = iWorldPos - camera_pos;
  o_depth = dot( camera_front.xyz, camera2wpos ) / camera_zfar;
}

//--------------------------------------------------------------------------------------
// This is the PS to be used when generating the shadow map
// It's different because we can NOT use the shadow test as we are rendering to the shadow map
// and can't read and write to the same RT
float4 PS_Shadows(
    float4 iPos : SV_POSITION
  , float2 iUV : TEXCOORD0
  , float4 iColor : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
) : SV_Target
{
  float4 texture_color = txAlbedo.Sample(samLinear, iUV) * iColor;

  if ( texture_color.a < 0.3 ) 
    discard;

  return float4(1,1,1,1);
}


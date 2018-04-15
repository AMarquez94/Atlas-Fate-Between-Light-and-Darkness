#include "common.fx"

//--------------------------------------------------------------------------------------
void VS(
  in float4 iPos     : POSITION
, in float3 iNormal  : NORMAL0
, in float2 iTex0    : TEXCOORD0
, in float2 iTex1    : TEXCOORD1
, in float4 iTangent : NORMAL1

, out float4 oPos      : SV_POSITION
, out float3 oNormal   : NORMAL0
, out float4 oTangent  : NORMAL1
, out float2 oTex0     : TEXCOORD0
, out float2 oTex1     : TEXCOORD1
, out float3 oWorldPos : TEXCOORD2
)
{
  float4 world_pos = mul(iPos, obj_world);

  // Rotar la normal segun la transform del objeto
  oNormal = mul(iNormal, (float3x3)obj_world);
  oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
  oTangent.w = iTangent.w;

  // Simulate a normal perturbation
  world_pos.xyz += oNormal * 0.2 * sin( 5 * global_world_time + world_pos.x + world_pos.z * 0.2);

  oPos = mul(world_pos, camera_view_proj);

  // Las uv's se pasan directamente al ps
  oTex0 = iTex0;
  oTex1 = iTex1;
  oWorldPos = world_pos.xyz;
}

float getSpecularTerm( float3 world_pos, float3 N, float3 L ) {
  // Specular term (aprox)
  float3 eye = normalize( camera_pos.xyz - world_pos );
  float3 eye_r = reflect( -eye, N );
  float3 H = normalize( eye + L );
  float cos_beta = saturate( dot( eye_r, L ));
  float specular_amount = pow( cos_beta, 50 );
  return specular_amount;
}

//--------------------------------------------------------------------------------------
float4 PS(
  float4 iPosition : SV_POSITION    // Screen coords
, float3 iNormal   : NORMAL0
, float4 iTangent  : NORMAL1
, float2 iTex0     : TEXCOORD0
, float2 iTex1     : TEXCOORD1
, float3 iWorldPos : TEXCOORD2
)  : SV_Target0
{

  int3 ss_load_coords = uint3(iPosition.xy, 0);

  // Obtain a random value associated to each pos in the surface
  float4 noise0 = txNoiseMap.Sample( samLinear, iTex0 * 2.0 + 0.8 * global_world_time * float2(.5,0)) * 2 - 1;      // -1..1
  float4 noise1 = txNoiseMap.Sample( samLinear, iTex0 * 8.0 + 0.7 * global_world_time * float2(.5,0.1)) * 2 - 1;      // -1..1
  float4 noise2 = txNoiseMap.Sample( samLinear, iTex0 * 16 + 0.7 * global_world_time * float2(.55,-0.123)) * 2 - 1;      // -1..1

  // Add 3 octaves to break pattern repetition
  float2 noiseF = noise0.xy + noise1.xy * 0.5 + noise2.xy * .25;

  // Modify the position where we take the sample
  float3 surface_pos = iWorldPos + noise0.xyz * .5;

  // Convert world coords to camera space in the range 0..1 to access the rt_main texture
  float4 pos_proj_space = mul( float4(surface_pos,1), camera_view_proj );
  float3 pos_homo_space = pos_proj_space.xyz / pos_proj_space.w;    // -1..1
  float2 pos_camera_unit_space = float2( ( 1 + pos_homo_space.x ) * 0.5, ( 1 - pos_homo_space.y ) * 0.5 );
  float4 color_base = txAlbedo.Sample(samClampLinear, pos_camera_unit_space) * obj_color.a;

  // Recover world position coords of the sea_bottom_wpos
  float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;
  float3 sea_bottom_wpos = getWorldCoords(iPosition.xy, zlinear);

  float4 water_color = obj_color;

  float  distance_of_water_traversed = length( sea_bottom_wpos - iWorldPos );
  float  amount_of_color_base = exp( -distance_of_water_traversed * 0.03 );
  float4 surface_color = lerp( water_color, color_base, amount_of_color_base );

  // Shadow factor entre 0 (totalmente en sombra) y 1 (no ocluido)
  float shadow_factor = computeShadowFactor( iWorldPos ); 
  float water_surface_shadow_factor = 0.5 + saturate( 0.5 + shadow_factor);

  // A random N in tangen space
  float3 normal_color = float3( noiseF.xy, 1.5 );
  float3x3 TBN = computeTBN( iNormal, iTangent );
  float3 N = normalize( mul( normal_color, TBN ) );

  // Compute reflected color using the eye direction reflected in the N
  float3 eye = normalize( iWorldPos - camera_pos );
  float3 eye_r = reflect( eye, iNormal );
  float4 env_color = txEnvironmentMap.SampleLevel( samLinear, eye_r, 3 );

  // Apply a small fresnel effect
  float  amount = dot( iNormal, -eye );
  amount = saturate ( amount );
  // Change the curve of the influence,.
  amount = pow( amount, 0.5 );

  // Add some specular
  float3 L = normalize( light_pos.xyz - iWorldPos );
  float spec_term = getSpecularTerm(iWorldPos, N, L);

  float4 final_color = surface_color * amount + env_color * ( 1 - amount );

  // Add spec + final color
  return final_color + spec_term * shadow_factor;
}

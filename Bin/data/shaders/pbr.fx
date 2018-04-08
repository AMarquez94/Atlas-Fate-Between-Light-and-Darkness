#include "common.fx"

#define PI 3.14159265359f


//--------------------------------------------------------------------------------------
// GBuffer generation pass. Vertex
//--------------------------------------------------------------------------------------
void VS_GBuffer(
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
  oPos = mul(world_pos, camera_view_proj);

  // Rotar la normal segun la transform del objeto
  oNormal = mul(iNormal, (float3x3)obj_world);
  oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
  oTangent.w = iTangent.w;

  // Las uv's se pasan directamente al ps
  oTex0 = iTex0;
  oTex1 = iTex1;
  oWorldPos = world_pos.xyz;
}

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Pixel shader
//--------------------------------------------------------------------------------------
void PS_GBuffer(
  float4 Pos       : SV_POSITION
, float3 iNormal   : NORMAL0
, float4 iTangent  : NORMAL1
, float2 iTex0     : TEXCOORD0
, float2 iTex1     : TEXCOORD1
, float3 iWorldPos : TEXCOORD2
, out float4 o_albedo : SV_Target0
, out float4 o_normal : SV_Target1
, out float1 o_depth  : SV_Target2
)
{
  o_albedo = txAlbedo.Sample(samLinear, iTex0);

  float3 N = computeNormalMap( iNormal, iTangent, iTex0 );
  o_normal = encodeNormal( N, 1 );

  // Compute the Z in linear space, and normalize it in the range 0...1
  // In the range z=0 to z=zFar of the camera (not zNear)
  float3 camera2wpos = iWorldPos - camera_pos;
  o_depth = dot( camera_front.xyz, camera2wpos ) / camera_zfar;
}

//--------------------------------------------------------------------------------------
void decodeGBuffer( 
     in float2 iPosition          // Screen coords
   , out float3 wPos 
   , out float3 N 
   , out float3 real_albedo
   ) {

  int3 ss_load_coords = uint3(iPosition.xy, 0);

  // Recover world position coords
  float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;
  //wPos = getWorldCoords(iPosition.xy, zlinear);

  // Recuperar la normal en ese pixel. Sabiendo que se
  // guardó en el rango 0..1 pero las normales se mueven
  // en el rango -1..1
  float4 N_rt = txGBufferNormals.Load(ss_load_coords);
  N = decodeNormal( N_rt.xyz );

  // Get other inputs from the GBuffer
  float4 albedo_rt = txGBufferAlbedos.Load(ss_load_coords);
  real_albedo = albedo_rt.xyz;
}

//--------------------------------------------------------------------------------------
// Ambient pass, to compute the ambient light of each pixel
float4 PS_ambient(
  in float4 iPosition : SV_Position
) : SV_Target
{
  // Will do something interesting here...
  return 0; // float4( 0.2, 0.2, 0.2, 0.f );
}

//--------------------------------------------------------------------------------------
// The geometry that approximates the light volume uses this shader
void VS_pass(
  in float4 iPos : POSITION
, in float3 iNormal : NORMAL0
, in float2 iTex0 : TEXCOORD0
, in float2 iTex1 : TEXCOORD0
, in float4 iTangent : NORMAL1
, out float4 oPos : SV_POSITION
) {
  float4 world_pos = mul(iPos, obj_world);
  oPos = mul(world_pos, camera_view_proj);
}

//--------------------------------------------------------------------------------------
// Simplified version of the textured.fx
float4 PS_dir_lights( in float4 iPosition : SV_Position ) : SV_Target
{
  float3 wPos, N, albedo;
  decodeGBuffer( iPosition.xy, wPos, N, albedo );

  float shadow_factor = computeShadowFactor( wPos );

  // Diffuse
  float3 Light = light_pos - wPos;
  Light = normalize( Light );
  float diffuseAmount = dot( N, Light );
  diffuseAmount = saturate( diffuseAmount );
  
  float4 light_amount = diffuseAmount * light_color * light_intensity * shadow_factor;

  return float4( light_amount.xyz, 1 );
}


//--------------------------------------------------------------------------------------
// Simplified version of the textured.fx
float4 PS_point_lights( in float4 iPosition : SV_Position ) : SV_Target
{
  float3 wPos, N, albedo;
  decodeGBuffer( iPosition.xy, wPos, N, albedo );

  // Diffuse
  float3 Light = light_pos - wPos;
  float  distance_to_light = length( Light );
  Light = normalize( Light );
  float diffuseAmount = dot( N, Light );
  diffuseAmount = saturate( diffuseAmount );

  float att_ratio = ( 1. - smoothstep( 0.90, 0.98, distance_to_light ));

  // Att per distance
  float att_factor = att_ratio / ( distance_to_light );

  float4 light_amount = diffuseAmount * light_color * light_intensity * att_factor;

  return float4( light_amount.xyz, 1 );
}

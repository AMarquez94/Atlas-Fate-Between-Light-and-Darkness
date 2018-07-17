#include "ctes.h"

//--------------------------------------------------------------------------------------
// from the object material
Texture2D    txAlbedo         SLOT(TS_ALBEDO);
Texture2D    txLightMap       SLOT(TS_LIGHTMAP);
Texture2D    txNormal         SLOT(TS_NORMAL);
Texture2D    txMetallic       SLOT(TS_METALLIC);
Texture2D    txRoughness      SLOT(TS_ROUGHNESS);
Texture2D    txEmissive       SLOT(TS_EMISSIVE);
Texture2D    txAOcclusion     SLOT(TS_AOCCLUSION);
Texture2D    txHeight         SLOT(TS_HEIGHT);
Texture2D    txNoiseMap       SLOT(TS_NOISE_MAP);
Texture2D    txNoiseMap2       SLOT(TS_NOISE_MAP2);
Texture3D    txLUT            SLOT(TS_LUT_COLOR_GRADING);

// from the light and env
Texture2D    txLightProjector SLOT(TS_LIGHT_PROJECTOR);
Texture2D    txLightShadowMap SLOT(TS_LIGHT_SHADOW_MAP);
Texture1DArray    txLightVolumeMap SLOT(TS_LIGHT_VOLUME_MAP);
TextureCube  txEnvironmentMap SLOT(TS_ENVIRONMENT_MAP);
TextureCube  txIrradianceMap  SLOT(TS_IRRADIANCE_MAP);

// output from deferred
Texture2D    txGBufferAlbedos     SLOT(TS_DEFERRED_ALBEDOS);
Texture2D    txGBufferNormals     SLOT(TS_DEFERRED_NORMALS);
Texture2D    txGBufferLinearDepth SLOT(TS_DEFERRED_LINEAR_DEPTH);
Texture2D    txAccLights          SLOT(TS_DEFERRED_ACC_LIGHTS);
Texture2D    txSelfIllum          SLOT(TS_DEFERRED_SELF_ILLUMINATION);
Texture2D    txAO                 SLOT(TS_DEFERRED_AO);

// 2nd material
Texture2D    txAlbedo1         SLOT( TS_ALBEDO1 );
Texture2D    txNormal1         SLOT( TS_NORMAL1 );
//Texture2D    txMetallic1       SLOT( (TS_FIRST_SLOT_MATERIAL_1 + TS_METALLIC) );
//Texture2D    txRoughness1      SLOT( (TS_FIRST_SLOT_MATERIAL_1 + TS_ROUGHNESS) );

// 3rd material
Texture2D    txAlbedo2         SLOT( TS_ALBEDO2 );
Texture2D    txNormal2         SLOT( TS_NORMAL2 );
//Texture2D    txMetallic2       SLOT( (TS_FIRST_SLOT_MATERIAL_2 + TS_METALLIC) );
//Texture2D    txRoughness2      SLOT( (TS_FIRST_SLOT_MATERIAL_2 + TS_ROUGHNESS) );

Texture2D    txMixBlendWeights SLOT( TS_MIX_BLEND_WEIGHTS );

// Bloom
Texture2D    txBloom0         SLOT(0);
Texture2D    txBloom1         SLOT(1);
Texture2D    txBloom2         SLOT(2);
Texture2D    txBloom3         SLOT(3);

//--------------------------------------------------------------------------------------
SamplerState samLinear        : register(s0);
SamplerState samBorderLinear  : register(s1);
SamplerComparisonState samPCF : register(s2);
SamplerState samClampLinear   : register(s3);
SamplerComparisonState samPCFWhite : register(s4);
SamplerState samClampBilinear   : register(s5);
SamplerState samClampPoint    : register(s6);
SamplerState samCount   			: register(s7);

//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
struct TInstanceWorldData {
  float4 InstanceWorld0 : TEXCOORD2;     // Stream 1
  float4 InstanceWorld1 : TEXCOORD3;    // Stream 1
  float4 InstanceWorld2 : TEXCOORD4;    // Stream 1
  float4 InstanceWorld3 : TEXCOORD5;    // Stream 1
};

// Build a World matrix from the instance information
float4x4 getWorldOfInstance( TInstanceWorldData d ) {
  return float4x4(d.InstanceWorld0, d.InstanceWorld1, d.InstanceWorld2, d.InstanceWorld3 );  
}

//--------------------------------------------------------------------------------------
float4x4 getSkinMtx(int4 iBones, float4 iWeights) {
  // This matrix will be reused for the position, Normal, Tangent, etc
  return  Bones[iBones.x] * iWeights.x
    + Bones[iBones.y] * iWeights.y
    + Bones[iBones.z] * iWeights.z
    + Bones[iBones.w] * iWeights.w;
}

//--------------------------------------------------------------------------------------
float2 hash2(float n) { return frac(sin(float2(n, n + 1.0))*float2(43758.5453123, 22578.1459123)); }

// ----------------------------------------
float shadowsTap(float2 homo_coord, float coord_z) {
  return txLightShadowMap.SampleCmp(samPCFWhite, homo_coord, coord_z, 0).x;
}

//--------------------------------------------------------------------------------------
float computeShadowFactor(float3 wPos) {

  // Convert pixel position in world space to light space
  float4 pos_in_light_proj_space = mul(float4(wPos, 1), light_view_proj_offset);
  float3 homo_space = pos_in_light_proj_space.xyz / pos_in_light_proj_space.w; // -1..1

  // Avoid the white band in the back side of the light
  if (pos_in_light_proj_space.z < 0.)
    return 0.f;

  // Poisson distribution random points around a circle
  const float2 offsets[] = {
    float2(0,0),
    float2(-0.3700152, 0.575369),
    float2(0.5462944, 0.5835142),
    float2(-0.4171277, -0.2965972),
    float2(-0.8671125, 0.4483297),
    float2(0.183309, 0.1595028),
    float2(0.6757001, -0.4031624),
    float2(0.8230421, 0.1482845),
    float2(0.1492012, 0.9389217),
    float2(-0.2219742, -0.7762423),
    float2(-0.9708459, -0.1171268),
    float2(0.2790326, -0.8920202)
  };

  // 1./ resolution * custom scale factor
  float coords_scale = light_shadows_step_with_inv_res;

  // Find a random rotation angle based on the world coords
  float angle = 2 * 3.14159268f * hash2(wPos.x + wPos.y).x;
  float cos_angle = cos(angle);
  float sin_angle = sin(angle);

  // Accumulate shadow taps
  float shadow_factor = 0;
	
	[unroll]
  for (int i = 0; i < 12; ++i) {

    // Get the random offset
    float2 coords = offsets[i];

    // Rotate the offset
    float2 rotated_coord = float2(
      coords.x * cos_angle - coords.y * sin_angle,
      coords.y * cos_angle + coords.x * sin_angle
      );
    // Scane and Translate
    float  tap_sample = shadowsTap(homo_space.xy + rotated_coord * coords_scale, homo_space.z);

    shadow_factor += tap_sample;
  }
  //return shadowsTap(homo_space.xy, homo_space.z);
  // Divide by the number of taps
  return shadow_factor / 12.f;
}

//--------------------------------------------------------------------------------------
float computeShadowFactorLight(float3 wPos) {

  // Convert pixel position in world space to light space
  float4 pos_in_light_proj_space = mul(float4(wPos, 1), light_view_proj_offset);
  float3 homo_space = pos_in_light_proj_space.xyz / pos_in_light_proj_space.w; // -1..1

  // Avoid the white band in the back side of the light
  if (pos_in_light_proj_space.z < 0.)
    return 0.f;

  return shadowsTap(homo_space.xy, homo_space.z);
}


float3x3 computeTBN(float3 inputN, float4 inputT) {

  // Prepare a 3x3 matrix to convert from tangent space to world space
  float3 N = inputN;
  float3 T = inputT.xyz;
  float3 B = cross(N, T) * inputT.w;
  return float3x3(T, B, N);
}

float3 computeNormalMap(float3 inputN, float4 inputT, float2 inUV) {

  // You might want to normalize input.N and input.T.xyz
  float3x3 TBN = computeTBN(inputN, inputT);

  // Normal map comes in the range 0..1. Recover it in the range -1..1
  float3 normal_color = txNormal.Sample(samLinear, inUV).xyz * 2.0 - 1.0;
  float3 wN = mul(normal_color, TBN);
  wN = normalize(wN);

  // Uncomment to disable normal map
  //wN = N;

  return wN;
}


// ------------------------------------------------------
// screen_coords va entre 0..1024
float3 getWorldCoords(float2 screen_coords, float zlinear_normalized) {

  /*
    // ux = -1 .. 1
    // Si screen_coords == 0 => ux = 1
    // Si screen_coords == 512 => ux = 0
    // Si screen_coords == 1024 => ux = -1
    float ux = 1.0 - screen_coords.x * camera_inv_resolution.x * 2;

    // Si screen_coords =   0 => uy = 1;
    // Si screen_coords = 400 => uy = 0;
    // Si screen_coords = 800 => uy = -1;
    float uy = 1.0 - screen_coords.y * camera_inv_resolution.y * 2;


    float3 view_dir2 = float3( ux * camera_tan_half_fov * camera_aspect_ratio
                , uy * camera_tan_half_fov
                , 1.) * ( zlinear_normalized * camera_zfar );

    float3 view_dir = mul( float4( screen_coords, 1, 1 ), camera_screen_to_world ).xyz;

    view_dir *= ( zlinear_normalized );

    float3 wPos =
      CameraFront.xyz * view_dir.z
    + CameraLeft.xyz  * view_dir.x
    + CameraUp.xyz    * view_dir.y
    + CameraWorldPos.xyz;
    return wPos;

    // camera_screen_to_world includes all the previous operations
  */

  float3 view_dir = mul(float4(screen_coords, 1, 1), camera_screen_to_world).xyz;
  return view_dir * zlinear_normalized + camera_pos;
}

// -----------------------------------------------------
// Converts range -1..1 to 0..1
float4 encodeNormal(float3 n, float nw) {
  return float4((n + 1.) * 0.5, nw);
}

// Converts range 0..1 to -1..1
float3 decodeNormal(float3 n) {
  return (n.xyz * 2. - 1.);
}

//--------------------------------------------------------------------------------------
void computeBlendWeights( float t1_a
                        , float t2_a
                        , float t3_a
                        , out float w1
                        , out float w2 
                        , out float w3 
                        ) {
  float depth = 0.05;
  float ma = max( t1_a, max( t2_a, t3_a ) ) - depth;
  float b1 = max( t1_a - ma, 0 );
  float b2 = max( t2_a - ma, 0 );
  float b3 = max( t3_a - ma, 0 );
  float b_total = b1 + b2 + b3;
  w1 = b1 / ( b_total );
  w2 = b2 / ( b_total );
  w3 = b3 / ( b_total );
}

// Projects pixel image on given world coordinate
float4 projectColor(float3 wPos) {

  // Convert pixel position in world space to light space
  float4 pos_in_light_proj_space = mul(float4(wPos, 1), light_view_proj_offset);
  float3 pos_in_light_homo_space = pos_in_light_proj_space.xyz / pos_in_light_proj_space.w; // -1..1

  // Use these coords to access the projector texture of the light dir
  //float2 t_uv = pos_in_light_homo_space.xy;
  //float distortionOffset = -global_world_time * 0.25;

  //float2 distort_uv = float2(t_uv.x + sin((t_uv.y + distortionOffset) * 20) * 0.05, t_uv.y + sin((t_uv.x + distortionOffset) * 20) * 0.05);
  float4 light_projector_color = txLightProjector.Sample(samBorderLinear, pos_in_light_homo_space.xy);

  if (pos_in_light_proj_space.z < 0.)
      light_projector_color = float4(0, 0, 0, 0); //return 1.f;

  // Fade to zero in the last 1% of the zbuffer of the light
  //light_projector_color *= smoothstep(1.0f, 0.15f, pos_in_light_homo_space.z);
  return light_projector_color;
}


// ----------------------------------------
float randNoise2D(float2 c){
  return frac(sin(dot(c.xy,float2(12.9898,78.233))) * 43758.5453);
}

float2 parallaxMappingB(float2 texCoords, float3 view_dir) {

  const float minLayers = 10.0;
  const float maxLayers = 15.0;
  float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), view_dir)));
  float layerDepth = 1.0 / numLayers;
  float currentLayerDepth = 0.0;
  float2 P = view_dir.xy * 0.175;
  float2 deltaTexCoords = P / numLayers;

  float2 currentTexCoords = texCoords;
  float currentDepthMapValue = 1 - txHeight.Sample(samLinear, currentTexCoords).r;

  [unroll(230)]
  while (currentLayerDepth < currentDepthMapValue)
  {
    currentTexCoords -= deltaTexCoords;
    currentDepthMapValue = 1 - txHeight.Sample(samLinear, currentTexCoords).r;
    currentLayerDepth += layerDepth;
  }

  float2 prevTexCoords = currentTexCoords + deltaTexCoords;
  float afterDepth = currentDepthMapValue - currentLayerDepth;
  float beforeDepth = (1 - txHeight.Sample(samLinear, prevTexCoords).r) - currentLayerDepth + layerDepth;
  float weight = afterDepth / (afterDepth - beforeDepth);
  float2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

  return finalTexCoords;
}

float2 parallaxMapping(float2 texCoords, float3 view_dir)
{
  // determine optimal number of layers
  const float minLayers = 10;
  const float maxLayers = 15;
  float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0, 0, 1), view_dir)));

  // height of each layer
  float layerHeight = 1.0 / numLayers;
  float curLayerHeight = 0.0;
  float2 dtex = 0.075 * view_dir.xy / numLayers;

  // current texture coordinates
  float2 currentTextureCoords = texCoords;
  float heightFromTexture = 1 - txHeight.Sample(samLinear, currentTextureCoords).r;

  // while point is above the surface
  [unroll(230)]
  while (heightFromTexture > curLayerHeight)
  {
    // to the next layer
    curLayerHeight += layerHeight;
    currentTextureCoords -= dtex;
    heightFromTexture = 1 - txHeight.Sample(samLinear, currentTextureCoords).r;
  }

  // previous texture coordinates
  float2 prevTCoords = currentTextureCoords + dtex;
  float nextH = heightFromTexture - curLayerHeight;
  float prevH = 1 - txHeight.Sample(samLinear, prevTCoords).r - curLayerHeight + layerHeight;

  float weight = nextH / (nextH - prevH);
  float2 finalTexCoords = prevTCoords * weight + currentTextureCoords * (1.0 - weight);
  //parallaxHeight = curLayerHeight + prevH * weight + nextH * (1.0 - weight);

  // return result
  return finalTexCoords;
}
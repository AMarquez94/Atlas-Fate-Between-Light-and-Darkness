//--------------------------------------------------------------------------------------
#include "ctes.h"

//--------------------------------------------------------------------------------------
Texture2D    txAlbedo         SLOT( TS_ALBEDO );
Texture2D    txLightMap       SLOT( TS_LIGHTMAP );

Texture2D    txLightProjector SLOT( TS_LIGHT_PROJECTOR );
Texture2D    txLightShadowMap SLOT( TS_LIGHT_SHADOW_MAP );

//--------------------------------------------------------------------------------------
SamplerState samLinear        : register(s0);
SamplerState samBorderLinear  : register(s1);
SamplerComparisonState samPCF : register(s2);

//--------------------------------------------------------------------------------------
float4x4 getSkinMtx( int4 iBones, float4 iWeights ) {
  // This matrix will be reused for the position, Normal, Tangent, etc
  return  Bones[iBones.x] * iWeights.x
        + Bones[iBones.y] * iWeights.y
        + Bones[iBones.z] * iWeights.z
        + Bones[iBones.w] * iWeights.w;
}

//--------------------------------------------------------------------------------------
float2 hash2( float n ) { return frac(sin(float2(n,n+1.0))*float2(43758.5453123,22578.1459123)); }

// ----------------------------------------
float shadowsTap( float2 homo_coord, float coord_z ) {
  return txLightShadowMap.SampleCmp(samPCF, homo_coord, coord_z, 0).x;
}

//--------------------------------------------------------------------------------------
float computeShadowFactor( float3 wPos ) {

  // Convert pixel position in world space to light space
  float4 pos_in_light_proj_space = mul( float4(wPos,1), light_view_proj_offset );
  float3 homo_space = pos_in_light_proj_space.xyz / pos_in_light_proj_space.w; // -1..1

  // Avoid the white band in the back side of the light
  if( pos_in_light_proj_space.z < 0. )
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
  float angle = 2 * 3.14159268f * hash2( wPos.x + wPos.y ).x;
  float cos_angle = cos( angle );
  float sin_angle = sin( angle );

  // Accumulate shadow taps
  float shadow_factor = 0;
  for( int i=0; i<12; ++i ) {

  	// Get the random offset
    float2 coords = offsets[i];

    // Rotate the offset
    float2 rotated_coord = float2( 
                              coords.x * cos_angle - coords.y * sin_angle,
                              coords.y * cos_angle + coords.x * sin_angle 
                              );
    // Scane and Translate
    float  tap_sample = shadowsTap( homo_space.xy + rotated_coord * coords_scale, homo_space.z );

    shadow_factor += tap_sample;
  }

  // Divide by the number of taps
  return shadow_factor / 12.f;
}


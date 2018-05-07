#include "common.fx"

// AO based on http://john-chapman-graphics.blogspot.com.es/2013/01/ssao-tutorial.html

// ----------------------------------------
float randNoise2D(float2 c){
  return frac(sin(dot(c.xy,float2(12.9898,78.233))) * 43758.5453);
}

// ----------------------------------------
void VS(
  in  float3 iPos   : POSITION     // x:0..1, y:0..1, z:0
, in  float4 iColor : COLOR0
, out float4 oPos   : SV_POSITION
, out float3 oUV    : TEXCOORD0
  )
{
  oPos = float4(iPos.x * 2 - 1., 1 - iPos.y * 2, 0.5, 1);

  // Generate the view dir directions in the 4 corners of the screen quad
  // 1 to the left, and -1 on the right
  oUV.x = -( 1 - ( iPos.x * 2.0 )) * camera_tan_half_fov * camera_aspect_ratio; 
  oUV.y =  ( 1 - ( iPos.y * 2.0 )) * camera_tan_half_fov; 
  oUV.z = -1.0;     // We are in the negative z axis of the camera
}

//--------------------------------------------------------------------------------------
float4 PS(
    in float4 iPosition : SV_Position
  , in float3 iUV : TEXCOORD0
) : SV_Target
{
  int3 ss_load_coords = uint3(iPosition.xy, 0);

  // Recover world position coords with a single mult!
  float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;
  float3 vPos = iUV * zlinear;

  // Get N in view space (was store in world coords)
  float4 N_rt = txGBufferNormals.Load(ss_load_coords);
  float3 N = decodeNormal( N_rt.xyz );
  N = mul( N, (float3x3) camera_view );
  // N = normalize( N );  // No visually required

  // Generate a TBN matrix from tangent space to view space
  float3 T = normalize( cross( N, float3(0,0,1) ) );
  float3 B = cross( N, T );
  float3x3 TBN = float3x3( T, B, N );

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

  // Find a random rotation angle based on the world coords
  float rand_val = randNoise2D(iPosition.xy);
  float angle = 2 * 3.14159268f * (rand_val);
  float cos_angle = cos( angle );
  float sin_angle = sin( angle );

  // We are moving very close to the camera
  const float amount_displaced = global_shared_fx_val1 / camera_zfar;
  const float zrange_discard = global_shared_fx_val2;
  const float amount_normal_z = global_shared_fx_val3;

  float occlusion = 0.0;
  for (int i = 0; i < 12; ++i) {

    // Get the random offset
    float2 coords = offsets[i];

    // Rotate the offset
    float2 rotated_coord = float2( 
                              coords.x * cos_angle - coords.y * sin_angle,
                              coords.y * cos_angle + coords.x * sin_angle 
                              );

    //rotated_coord = coords;

    // Generate an offset in Tangent space and bring it to view space
    float3 offset_tbn = float3( rotated_coord, amount_normal_z );
    offset_tbn = normalize( offset_tbn );
    float3 delta_view_space = mul( offset_tbn, TBN );
    //return dot( delta_tbn_space, float3(0,0,1));

    // Move along the noised N in view space 
    float3 sample_view_pos = vPos + delta_view_space * amount_displaced;

    // Obtain the linear Z of the new sample in view space
    float sample_linear_depth = -sample_view_pos.z;

    // Project in homo space including offset to get values in the range 0..1
    float4 sample_proj = mul( float4( sample_view_pos, 1), camera_proj_with_offset );
    sample_proj.xy /= sample_proj.w;

    // Read existing depth at the projected coord
    float prevDepthAtSampleCoords = txGBufferLinearDepth.Sample(samClampLinear, sample_proj.xy).r;

    // This is to prevent large Z range to generate occlusion... will remove black halos in the character head
    float rangeCheck = smoothstep( zrange_discard, 0, abs(zlinear - prevDepthAtSampleCoords) );
    occlusion += ( prevDepthAtSampleCoords <= sample_linear_depth ? 1.0 : 0.0) * rangeCheck;
    //occlusion += ( smoothstep( 0.002, 0.0, prevDepthAtSampleCoords - sample_linear_depth )) * rangeCheck;
  }

  // Module the amount of occlusion
  return ( 1 - global_shared_fx_amount ) + (1 - occlusion / 12 ) * global_shared_fx_amount;
}

#include "common.fx"

// AO based on http://john-chapman-graphics.blogspot.com.es/2013/01/ssao-tutorial.html

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
		float2(-0.94201624, -0.39906216),
		float2(0.94558609, -0.76890725),
		float2(-0.094184101, -0.92938870),
		float2(0.34495938, 0.29387760),
		float2(-0.91588581, 0.45771432),
		float2(-0.81544232, -0.87912464),
		float2(-0.38277543, 0.27676845),
		float2(0.97484398, 0.75648379),
		float2(0.44323325, -0.97511554),
		float2(0.53742981, -0.47373420),
		float2(-0.26496911, -0.41893023),
		float2(0.79197514, 0.19090188),
		float2(-0.24188840, 0.99706507),
		float2(-0.81409955, 0.91437590),
		float2(0.19984126, 0.78641367),
		float2(0.14383161, -0.14100790)
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

	  float OccRadiusNorm = clamp((zrange_discard - camera_znear) / (camera_zfar - camera_znear), 0, 1);
		
  float occlusion = 0.0;
  for (int i = 0; i < 16; ++i) {

    // Get the random offset
    float2 coords = offsets[i];

    // Rotate the offset
    float2 rotated_coord = float2( 
                              coords.x * cos_angle - coords.y * sin_angle,
                              coords.y * cos_angle + coords.x * sin_angle 
                              );
															
    // Generate an offset in Tangent space and bring it to view space
    float3 offset_tbn = float3( rotated_coord, amount_normal_z );
    offset_tbn = normalize( offset_tbn );
    float3 delta_view_space = mul( offset_tbn, TBN );
    float3 sample_view_pos = vPos + delta_view_space * amount_displaced;

    // Obtain the linear Z of the new sample in view space
    float sample_linear_depth = -sample_view_pos.z;
    float4 sample_proj = mul( float4( sample_view_pos, 1), camera_proj_with_offset );
    sample_proj.xy /= sample_proj.w;

    // Read existing depth at the projected coord
    float prevDepthAtSampleCoords = txGBufferLinearDepth.Sample(samClampLinear, sample_proj.xy).r;

    // This is to prevent large Z range to generate occlusion... will remove black halos in the character head
    float rangeCheck = smoothstep(zrange_discard, 1.0,  OccRadiusNorm / abs(zlinear - prevDepthAtSampleCoords));
    occlusion += ( prevDepthAtSampleCoords <= sample_linear_depth ? 1.0 : 0.0) * rangeCheck;
  }

	//return (1 - occlusion / 16 );
  // Module the amount of occlusion
  return ( 1 - global_shared_fx_amount ) + (1 - occlusion / 16 ) * global_shared_fx_amount;
}

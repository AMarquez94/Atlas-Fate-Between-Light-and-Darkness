#include "common.fx"

struct TInstanceLight {
  float4 InstanceWorld0 : TEXCOORD2;     // Stream 1
  float4 InstanceWorld1 : TEXCOORD3;    // Stream 1
  float4 InstanceWorld2 : TEXCOORD4;    // Stream 1
  float4 InstanceWorld3 : TEXCOORD5;    // Stream 1
};

struct TInstanceShadow {
  float4 InstanceWorld0 : TEXCOORD9;     // Stream 1
  float4 InstanceWorld1 : TEXCOORD10;    // Stream 1
  float4 InstanceWorld2 : TEXCOORD11;    // Stream 1
  float4 InstanceWorld3 : TEXCOORD12;    // Stream 1
};

//--------------------------------------------------------------------------------------
// Light volume generation pass. Vertex
//--------------------------------------------------------------------------------------
void VS_IVLight(
    in float4 iPos     : POSITION
    , in float3 iNormal : NORMAL0
    , in float2 iTex0 : TEXCOORD0
    , in float2 iTex1 : TEXCOORD1
    , in float4 iTangent : NORMAL1
  // From stream 1 we read the instance information 
		, in TInstanceWorldData instance_data     // Stream 1
		, in float4 InstanceLightPos   : TEXCOORD6   // TimeToLife, TimeBlendingOut, ...  
		, in float4 InstanceLightDir   : TEXCOORD7   // TimeToLife, TimeBlendingOut, ...  
		, in float4 InstanceLightValues   : TEXCOORD8   // TimeToLife, TimeBlendingOut, ...  
		, in TInstanceShadow instance_data_projection     // Stream 1
					
    , out float4 oPos : SV_POSITION
    , out float3 oNormal : NORMAL0
    , out float4 oTangent : NORMAL1
    , out float2 oTex0 : TEXCOORD0
    , out float2 oTex1 : TEXCOORD1
    , out float3 oWorldPos : TEXCOORD2
		, out float4 oLightPos : TEXCOORD3
		, out float4 oLightDir : TEXCOORD4
		, out float4 oLighValues : TEXCOORD5
		, out float4x4 oViewProj: TInstanceLight
)
{
	float4x4 instance_world = getWorldOfInstance(instance_data);
	oViewProj = float4x4(instance_data_projection.InstanceWorld0, 
											instance_data_projection.InstanceWorld1, 
											instance_data_projection.InstanceWorld2, 
											instance_data_projection.InstanceWorld3 );//getWorldOfInstance(instance_data_projection);

	float4 world_pos = mul(iPos, instance_world);
	oPos = mul(world_pos, camera_view_proj);

	// Rotar la normal segun la transform del objeto
	oNormal = mul(iNormal, (float3x3)instance_world);
	oTangent.xyz = mul(iTangent.xyz, (float3x3)instance_world);
	oTangent.w = iTangent.w;

	// Las uv's se pasan directamente al ps
	oTex0 = iTex0;
	oTex1 = iTex1;
	oWorldPos = world_pos.xyz;
	oLightPos = InstanceLightPos;
	oLightDir = InstanceLightDir;
	oLighValues = InstanceLightValues;
}

float computeVolumeShadowFactorLight(float4x4 mat, float3 wPos) {

  // Convert pixel position in world space to light space
  float4 pos_in_light_proj_space = mul(float4(wPos, 1), mat);
  float3 homo_space = pos_in_light_proj_space.xyz / pos_in_light_proj_space.w; // -1..1

  // Avoid the white band in the back side of the light
  if (pos_in_light_proj_space.z < 0.)
    return 0.f;
		
	return txLightShadowMap.SampleCmp(samPCFWhite, homo_space.xy, homo_space.z, 0).x;
		
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
// Light volume generation pass. Fragment
//--------------------------------------------------------------------------------------
float4 PS_IVLight( 
    in float4 iPos : SV_POSITION
    , float3 iNormal : NORMAL0
    , float4 iTangent : NORMAL1
    , float2 iTex0 : TEXCOORD0
    , float2 iTex1 : TEXCOORD1
    , float3 iWorldPos : TEXCOORD2
		, float4 iLightPos : TEXCOORD3
		, float4 iLightDir : TEXCOORD4
		, float4 iLightValues : TEXCOORD5
		, float4x4 iViewProj: TInstanceLight
) : SV_Target
{
    // Sampling planes volumetric lights based shader.
		float shadow_factor = computeVolumeShadowFactorLight(iViewProj, iWorldPos);

    float camera_dist = length(iWorldPos - iLightPos.xyz);
    float val = 1 / (0.45 + (camera_dist * camera_dist) * iLightValues.w);

    // From wPos to Light
    float3 light_dir_full = iLightPos.xyz - iWorldPos;
    float  distance_to_light = length(light_dir_full);
    float3 light_dir = light_dir_full / distance_to_light;
    float4 noise0 = txNoiseMap.Sample(samLinear, iTex0 * 1.0 + 0.06 * global_world_time * float2(.5, 0));
		
    float theta = dot(light_dir, -iLightDir.xyz);
    float att_spot = clamp((theta - iLightValues.z) / 0.18, 0, 1);
    float clamp_spot = theta > iLightValues.x ? att_spot : 0.0; // spot factor 
		
    return float4(light_color.xyz, clamp_spot * val * noise0.x) * shadow_factor;// * projectColor(iWorldPos);
}

#define FACTOR_TAU 0.0001f
#define FACTOR_PHI 100000.0f
#define PI_RCP 1.318309388618379067153776752674503f

float4 PS_GBuffer_RayShafts(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
): SV_Target0
{
	float NB_STEPS = 40;
  float TAU = FACTOR_TAU * 25.1;
  float PHI = FACTOR_PHI * light_intensity * 4;
	
	// Clamped world position to closest fragment
	int3   ss_load_coords = uint3(Pos.xy, 0);
  float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;
  float  depth = zlinear > Pos.z ? Pos.z : zlinear;
  float3 wPos = getWorldCoords(Pos.xy, depth);
	
  float3 ray_vector = camera_pos - wPos;
	float ray_length = length(ray_vector);
	float3 ray_dir = ray_vector / ray_length;

	float step_length = ray_length / NB_STEPS;
	float3 step = ray_dir * step_length;

  float4x4 ditherPattern = {{ 0.0f, 0.5f, 0.125f, 0.625f},
                            { 0.75f, 0.22f, 0.875f, 0.375f},
                            { 0.1875f, 0.6875f, 0.0625f, 0.5625},
                            { 0.9375f, 0.4375f, 0.8125f, 0.3125}};
  float ditherValue = ditherPattern[Pos.x % 4][Pos.y % 4];
	
	float3 currentPosition = wPos + step * ditherValue;
	float total_fog = 0.0f;
	float l = ray_length;
			
	for (int i = 0; i < NB_STEPS; i++)
	{
		float shadowTerm = computeShadowFactorLight(currentPosition);
    float d = length(currentPosition - light_pos);
    float dRCP = rcp(d);
    float amount = TAU*(shadowTerm*(PHI*0.25f*PI_RCP)*dRCP*dRCP)*exp(-d*TAU)*exp(-l*TAU)*step_length;
		
		l-=step_length;
    total_fog += amount;
		currentPosition += step;
	}

	return float4(light_color.xyz * total_fog, 1);
}

float4 PS_GBuffer_Shafts(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
): SV_Target0
{
	// Fresnel component
	float3 dir_to_eye = normalize(camera_pos.xyz - iWorldPos.xyz);
	float3 N = normalize(iNormal.xyz);
	float fresnel = dot(N, -dir_to_eye);

	float4 color = float4(0.8, 0.8, 0.8, 1);	
	color.a = txAlbedo.Sample(samLinear, iTex0).r;
	
	color.a *= txNoiseMap.Sample(samLinear, iTex0 * 1.0 + 0.02 * global_world_time * float2(.5, 0)).r;
	color.a *= txNoiseMap.Sample(samLinear, iTex0 * 1.0 - 0.04 * global_world_time * float2(.5, 0));
	
	// Compute smooth intersections
	int3 ss_load_coords = uint3(Pos.xy, 0);
	float linear_depth = txGBufferLinearDepth.Load(ss_load_coords).x;
	float fragment_depth = dot(iWorldPos - camera_pos, camera_front) / camera_zfar;
	
	float delta_c = length(camera_pos - iWorldPos);
	float delta_z = abs(linear_depth - fragment_depth);
	color.a *= saturate(delta_z * camera_zfar);
	color.a *= 1 - saturate(1/(delta_c * delta_c));
	
	//float theta = dot(out_lightdir, -light_direction.xyz);
	//float att_spot = clamp((theta - light_outer_cut) / (light_inner_cut - light_outer_cut), 0, 1);
	//float clamp_spot = theta > light_angle ? 1.0 * att_spot : 0.0; // spot factor 
	
	color.a *= pow(abs(fresnel),4) * 0.1;
	return color;
}

float4 PS_GBuffer_Beam(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
): SV_Target0
{
	float4 color = float4(0.8, 0.8, 0.8, 1);	
	color.a = txAlbedo.Sample(samLinear, iTex0).r;
	color.a *= txNoiseMap.Sample(samLinear, iTex0 * 1.0 + 0.1 * global_world_time * float2(.5, 0)).r;
	color.a *= txNoiseMap.Sample(samLinear, iTex0 * 1.0 - 0.1 * global_world_time * float2(.5, 0)).r;
	
	return 1 * color * (0.65 + sin(global_world_time * 2) * .15);
}

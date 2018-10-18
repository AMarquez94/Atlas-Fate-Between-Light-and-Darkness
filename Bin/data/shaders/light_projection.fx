#include "common.fx"
// Variable declaration
  
struct VS_OUTPUT_BASIC
{
  float4 Pos : SV_POSITION;
  float3 N    : NORMAL0;
  float2 UV   : TEXCOORD0;
  float3 wPos : TEXCOORD1;
  float4 T    : NORMAL1;
};

//--------------------------------------------------------------------------------------
void decodeGBuffer(
	in float2 iPosition          // Screen coords
	, out float3 wPos
	, out float3 N
	, out float3 real_albedo
	, out float3 real_specular_color
	, out float  roughness
	, out float3 reflected_dir
	, out float3 view_dir
) {

	int3 ss_load_coords = uint3(iPosition.xy, 0);

	// Recover world position coords
	float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;
	wPos = getWorldCoords(iPosition.xy, zlinear);

	// Recuperar la normal en ese pixel. Sabiendo que se
	// guardó en el rango 0..1 pero las normales se mueven
	// en el rango -1..1
	float4 N_rt = txGBufferNormals.Load(ss_load_coords);
	N = decodeNormal(N_rt.xyz);
	N = normalize(N);

	// Get other inputs from the GBuffer
  float4 albedo = txGBufferAlbedos.Load(ss_load_coords);
	// In the alpha of the albedo, we stored the metallic value
	// and in the alpha of the normal, we stored the roughness
	float  metallic = albedo.a;
	roughness = N_rt.a;

	// Apply gamma correction to albedo to bring it back to linear.
  albedo.rgb = pow(albedo.rgb, 2.2f);// *projectColor(wPos).xyz;

	// Lerp with metallic value to find the good diffuse and specular.
	// If metallic = 0, albedo is the albedo, if metallic = 1, the
	// used albedo is almost black
  real_albedo = albedo.rgb * (1. - metallic);

	// 0.03 default specular value for dielectric.
  real_specular_color = lerp(0.03f, albedo.rgb, metallic);

	// Eye to object
	float3 incident_dir = normalize(wPos - camera_pos.xyz);
	reflected_dir = normalize(reflect(incident_dir, N));
	view_dir = -incident_dir;
}

//--------------------------------------------------------------------------------------
// The geometry that approximates the light volume uses this shader
void VS_pass(
	in float4 iPos : POSITION
	, in float3 iNormal : NORMAL0
	, in float2 iTex0 : TEXCOORD0
	, in float2 iTex1 : TEXCOORD0
	, in float4 iTangent : NORMAL1
	, out float4 oPos : SV_POSITION0
) {
	float4 world_pos = mul(iPos, obj_world);
	oPos = mul(world_pos, camera_view_proj);
}

// Projects pixel image on given world coordinate
float4 projectWavedShadow(float3 wPos) {

  // Convert pixel position in world space to light space
  float4 pos_in_light_proj_space = mul(float4(wPos, 1), light_view_proj_offset);
  float3 pos_in_light_homo_space = pos_in_light_proj_space.xyz / pos_in_light_proj_space.w; // -1..1

  // Use these coords to access the projector texture of the light dir
  float2 t_uv = pos_in_light_homo_space.xy;
  float distortionOffset = -global_world_time * 0.25;
	//float dist = pow(distance(wPos, player_next_pos), 0.3);
	
	float2 distort_uv = float2(t_uv.x + sin((t_uv.x - distortionOffset) * 20) * 0.025, t_uv.y 
										+ 6 * sin((3 * t_uv.x - 46 * distortionOffset)) * 0.0035 * 6 * global_player_speed);
		
  //float2 distort_uv = float2(t_uv.x + sin((t_uv.y + distortionOffset) * 20) * 0.025, t_uv.y + sin((t_uv.x - distortionOffset * 2) * 20) * 0.025);
	float4 noise0 = txNoiseMap.Sample(samBorderLinear, distort_uv).r; 
	float4 noise1 = txNoiseMap2.Sample(samBorderLinear, distort_uv).r * 1.5; 
	float4 value = lerp(noise0, noise1, global_player_speed);
	
  float4 light_projector_color = txLightProjector.Sample(samBorderLinear, distort_uv);
  //light_projector_color *= (txNoiseMap.Sample(samBorderLinear, distort_uv).r); 
	light_projector_color *=  noise1; 

	// Fade to zero in the last 1% of the zbuffer of the light
  light_projector_color *= smoothstep(1.f, 0.09f, pos_in_light_homo_space.z);
  return light_projector_color;
}

float4 PS_Projection(in float4 iPosition : SV_Position) : SV_Target
{
	float3 wPos, N, albedo, specular_color, reflected_dir, view_dir;
	float  roughness;
	decodeGBuffer(iPosition.xy, wPos, N, albedo, specular_color, roughness, reflected_dir, view_dir);
		
	// From wPos to Light
	float3 light_dir_full = light_pos.xyz - wPos;
	float  distance_to_light = length(light_dir_full);
	float  att = (1. - smoothstep(0.40, 0.98, distance_to_light / light_radius)); // Att, point light
	
	return float4(0,0,0,1) *projectWavedShadow(wPos);
}
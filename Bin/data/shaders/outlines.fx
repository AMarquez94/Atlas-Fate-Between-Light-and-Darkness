//--------------------------------------------------------------------------------------
#include "common.fx"

// https://stackoverflow.com/questions/34601325/directx11-read-stencil-bit-from-compute-shader
Texture2D<uint2>    txBackBufferStencil        SLOT(TS_ALBEDO);


// In case we want to use the stencil test, we must have the zbuffer+stencil from the backbuffer
// actived when rendering the outlines pass
// But we can't read the current values as it's 
// In the tech 'outlines' add this ps entry point instead of the default PS
//    "ps_entry_point": "PS_Solid",

float4 PS_Solid() : SV_Target
{
  return float4(1,0.0,0,0.0);
}

float4 ComputeBars(float2 iTex0){

	return 1 - saturate(round(abs(frac(iTex0.x * 100) * 2)));
}

float2 ComputeScreenShock(float2 iTex0)
{
	float3 shock_params = float3(10, 0.8, 0.1);
	float dist = distance(iTex0, float2(0.5,0.5));

	if ((dist <= (linear_time + shock_params.z)) && (dist >= (linear_time - shock_params.z))) 
	{
		float diff = (dist - linear_time);
		float pow_diff = 1.0 - pow(abs(diff * shock_params.x), shock_params.y); 
		float diff_time = diff  * pow_diff; 
		float2 diff_uv = normalize(iTex0 - float2(0.5,0.5)); 
		return iTex0 + (diff_uv * diff_time);
	} 
	
	return iTex0;
}

float4 ComputeBitMap(uint s_cc, float2 uv)
{
		float4 glitch = txNoiseMap2.Sample(samClampPoint, uv);	
	if(s_cc == 0xF4) return float4(1,0,0,1);
	if(s_cc == 0xFF) return float4(0,1,1,1);
	
	return float4(0,0,0,0);
}

float4 ComputeOutline(float4 color, int3 ss_load_coords, float2 uv)
{
	uint s_cc = txBackBufferStencil.Load(ss_load_coords).g;
	float4 outline_color = txOutlines.Load(ss_load_coords);
	float4 s_color = ComputeBitMap(s_cc, uv);
	
	// n = north, s = south, e = east, w = west, c = center
	uint s_nw = txBackBufferStencil.Load(ss_load_coords + int3(1, -1, 0)).y;
	uint s_nc = txBackBufferStencil.Load(ss_load_coords + int3(0, -1, 0)).y;
	uint s_ne = txBackBufferStencil.Load(ss_load_coords + int3(-1, -1, 0)).y;
	uint s_cw = txBackBufferStencil.Load(ss_load_coords + int3(1, 0, 0)).y;

	uint s_ce = txBackBufferStencil.Load(ss_load_coords + int3(-1, 0, 0)).y;
	uint s_sw = txBackBufferStencil.Load(ss_load_coords + int3(1, 1, 0)).y;
	uint s_sc = txBackBufferStencil.Load(ss_load_coords + int3(0, 1, 0)).y;
	uint s_se = txBackBufferStencil.Load(ss_load_coords + int3(-1, 1, 0)).y;

	// We expect all the 3x3 pixels to have the same value 
	int sum_stencils = s_nw + s_nc + s_ne + s_cw + s_cc + s_ce + s_sw + s_sc + s_se;
	uint diff = sum_stencils - s_cc * 9;
	
	if(diff != 0)
	{	
		//float2 newpos = float2(iTex0.y, nrand(iTex0.x, iTex0.y));	
		//float4 glitch = txNoiseMap2.Sample(samClampPoint, ss_load_coords.xy);	
		return float4(0, 1, 1, 0.75) * outline_alpha;
	}
		
	if(s_cc != 0)
	{
		//float4 in_color = txAlbedo2.Sample(samLinear, iTex0.xy);
		//float2 newpos = float2(iTex0.y, nrand(iTex0.x, iTex0.y));
		//float4 glitch = txNoiseMap2.Sample(samClampPoint, depth.xx);	
		return s_color * float4(1, 1, 1, 1) * outline_alpha * float4(outline_color.xyz,0.5);
	}
		
	return float4(color.xyz, 0.5) * outline_alpha;
}

float4 PS_PostFX_Wave(float4 iPosition : SV_POSITION, float2 UV : TEXCOORD0) : SV_Target
{
	int3 ss_load_coords = uint3(iPosition.xy, 0);

	UV = ComputeScreenShock(UV);
	return txAlbedo.Sample(samLinear, UV);
}

//--------------------------------------------------------------------------------------
float4 PS(float4 iPosition : SV_POSITION, float2 UV : TEXCOORD0) : SV_Target
{ 
	float edge_strength = 4.f;
	float pulse_strength = 0.1f;
   
	// Retrieve the linear depth on given pixel
	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float depth = txGBufferLinearDepth.Load(ss_load_coords).x;
	float4 outline_color = txOutlines.Load(ss_load_coords);
	//float3 wPos = getWorldCoords(iPosition.xy, depth);
		
	float average = 0.125f * (
	txGBufferLinearDepth.Load(ss_load_coords + int3(1,-1,0)).x
	+ txGBufferLinearDepth.Load(ss_load_coords + int3(0,-1,0)).x
	+ txGBufferLinearDepth.Load(ss_load_coords + int3(-1,-1,0)).x
	+ txGBufferLinearDepth.Load(ss_load_coords + int3(1,0,0)).x	
	+ txGBufferLinearDepth.Load(ss_load_coords + int3(-1,0,0)).x
	+ txGBufferLinearDepth.Load(ss_load_coords + int3(1, 1,0)).x
	+ txGBufferLinearDepth.Load(ss_load_coords + int3(0, 1,0)).x
	+ txGBufferLinearDepth.Load(ss_load_coords + int3(-1, 1,0)).x);
	
	// To resolve a problem with depth values (remove if not needed)
	float edge = sqrt(abs(depth - average)) * edge_strength;
	if(edge > 0.5) discard;
	//edge = clamp(edge, 0, 0.5);

	depth = saturate(2.0f * depth);
	depth = 1 - depth;
	depth *= depth;
	depth = 1 - depth;

	//float tPos = length(wPos - camera_pos) / (camera_zfar);
	float2 samplePos = float2(1.0f * depth.xx);
	samplePos.x -= pulse_strength * linear_time;

	float4 band = txNoiseMap.Sample(samLinear, samplePos);
	//band *= ComputeBars(samplePos) * band;
	band *= (band * (2.0f + edge * 30.0f) + edge * 5.0f);
	
	if (pulse_strength * (linear_time/alive_time) > (outline_color.a))
		return ComputeOutline(band, ss_load_coords, UV * samplePos.x);
	
	// or we are outside, all zeros.
	return float4(band.xyz, 0.5) * outline_alpha;
}

float4 PS_Environment(float4 iPosition : SV_POSITION, float2 UV : TEXCOORD0) : SV_Target
{
	int3 ss_load_coords = uint3(iPosition.xy, 0);
 	uint s_cc = txBackBufferStencil.Load(ss_load_coords).g;
	float4 color = txEmissive.Sample(samClampLinear, UV);
		
	float4 noise0 = txNoiseMap.Sample( samLinear, UV * 1.0 + 0.4 * global_world_time/3 * float2(.5,0)) * 2 - 1;      // -1..1
  float4 noise1 = txNoiseMap.Sample( samLinear, UV * 2.0 + 0.5 * global_world_time/3 * float2(.5,0.1)) * 2 - 1;      // -1..1
  float4 noise2 = txNoiseMap.Sample( samLinear, UV * 4 + 0.5 * global_world_time/3 * float2(.55,-0.123)) * 2 - 1;      // -1..1
	float4 noiset = txNoiseMap2.Sample( samLinear, UV * 1.0 + 0.4 * global_world_time/10 * float2(0,1)) * 2 - 1; 
	float4 noiset2 = txNoiseMap2.Sample( samLinear, UV * 1.0 + 0.4 * global_world_time/10 * float2(0,1)) * 2 - 1; 
	float2 noiseF = noise0.xy * 0.35 + noise1.xy * 0.25 + noise2.xy * .125;
	float2 final_uv = UV + (noiseF * 0.01 * noiset * 4.35 * noiset2) * player_shadowed;
	
	if(s_cc == 240) return txEmissive.Sample( samClampLinear, final_uv);
	
	return color;
}
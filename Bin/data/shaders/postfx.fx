#include "common.fx"

// PostFX contrast
float4 postfx_contrast(float4 color)
{
	const float3 lum_coeff = float3(0.2125, 0.7154, 0.0721);

	float3 avg_lum = float3(1, 1, 1);
	float dotval = dot(color.xyz, lum_coeff);
	float3 intensity = float3(dotval, dotval, dotval);

	float3 sat_color = lerp(intensity, color.xyz, 1.);
	float3 con_color = lerp(avg_lum, sat_color, 1.05);

	return float4(con_color, 1);
}

float4 PS_PostFX_Vignette(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	float4 color = txAlbedo.Sample(samClampLinear, iTex0); 
  float2 position = (iPosition.xy * camera_inv_resolution) - float2(0.5f,0.5f);
  //position.x *= camera_aspect_ratio;

  float len = length(position);
	float vignette = smoothstep(0.95, 0.95 - 0.65, len);
  color.rgb = lerp(color.rgb, color.rgb * vignette, 0.75);

	return color;
}

// PostFX Exponential distance fog.
float4 PS_PostFX_ExpFog(float4 iPosition, float2 iTex0, float4 in_color)
{
	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float depth = txGBufferLinearDepth.Load(ss_load_coords).x;
	float3 wPos = getWorldCoords(iPosition.xy, depth);
  //float4 in_color = txAlbedo.Sample(samClampLinear, iTex0.xy);
	
	float3 frag_dir = (wPos - camera_pos.xyz);
	float dist = abs(length(frag_dir));
	float3 tmp_color = float3(0.188, 0.266, 0.717);

	float fog_factor = exp( (dist * -global_fog_density * .75)* (dist* global_fog_density * .75));	
	//fog_factor = fog_factor * (1 - smoothstep(0.98, 1.1, depth));
	if(depth > 0.99) fog_factor = 0.95;
	
	float4 final_color = lerp(float4(tmp_color,1), in_color, fog_factor);
		
	return float4(final_color);
}

// PostFX Exponential ground distance fog.
float4 PS_PostFXFog(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float depth = txGBufferLinearDepth.Load(ss_load_coords).x;
	float3 wPos = getWorldCoords(iPosition.xy, depth);
  float4 in_color = txAlbedo.Sample(samClampLinear, iTex0.xy);
	
	float3 frag_dir = (wPos - camera_pos.xyz);
	float dist = abs(length(frag_dir));

	//You have to tweak this values
	float be = 0.0045 * smoothstep(0.0, 2.0, 10.0 - wPos.y);
	float bi = 0.375* smoothstep(0.0, 80, 10.0 - wPos.y);
	 
	float ext = exp(-dist * be);
	float insc = exp(-dist * bi);
	float4 final_color = in_color + ext * float4(global_fog_color,1) * (1 - insc);
	
	return PS_PostFX_ExpFog(iPosition, iTex0, final_color);
	
	return in_color + ext * float4(global_fog_color,1) * (1 - insc);
}

float2 shiftChannel(float2 iTex0, float value, float shift) 
{
	float r2 = (iTex0.x - postfx_cs_offset) * (iTex0.x - postfx_cs_offset) + (iTex0.y - postfx_cs_offset) * (iTex0.y - postfx_cs_offset);
	float f = 1 + r2 * (value + shift * sqrt(r2));

	float x = f*(iTex0.x - postfx_cs_offset) + postfx_cs_offset;
	float y = f*(iTex0.y - postfx_cs_offset) + postfx_cs_offset;

	return float2(x, y);
}

// PostFX Chromatic Aberration
float4 PS_PostFX_CA(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{

	float distortion = txNoiseMap.Sample( samLinear, iTex0 * 0.0001).r;  
		
	float2 bDist = shiftChannel(iTex0, -postfx_ca_offset, postfx_ca_offset);
	float2 gDist = shiftChannel(iTex0, -postfx_ca_offset - postfx_ca_amount, postfx_ca_offset + postfx_ca_amount);
	float2 rDist = shiftChannel(iTex0, -postfx_ca_offset - postfx_ca_amount * 2, postfx_ca_offset + postfx_ca_amount * 2);
  
	float4 distorsion_r = txAlbedo.Sample(samClampLinear, iTex0 + distortion);
	float4 distorsion_g = txAlbedo.Sample(samClampLinear, iTex0 + distortion);
	float4 distorsion_b = txAlbedo.Sample(samClampLinear, iTex0 + distortion);

	return txAlbedo.Sample(samClampLinear, iTex0);
  return float4(distorsion_r.r, distorsion_g.g, distorsion_b.b, 1);
}

// Adaptative lighting methods

float NUM_SAMPLES = 10;
float Density = 1;

float4 PS_PostFX_Sharpen(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	const float kernel[9] = { -1, -1, -1,
				-1, 9, -1,
				-1, -1, -1 };
				
	const float step_w = camera_inv_resolution.x;
	const float step_h = camera_inv_resolution.y;

	const float2 offset[9] = { float2(-step_w, -step_h), float2(0.0, -step_h), float2(step_w, -step_h), 
				float2(-step_w, 0.0), float2(0.0, 0.0), float2(step_w, 0.0), 
				float2(-step_w, step_h), float2(0.0, step_h), float2(step_w, step_h) };

	int i = 0;
  float4 sum = float4(0, 0, 0, 0);
   
	for( i=0; i<9; i++ )
	{
		float4 tmp = txAlbedo.Sample(samLinear, iTex0.xy + offset[i] * .1);
		sum += tmp * kernel[i];
	}
  
  return sum;
}

float4 PS_PostFX_LScattering( in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	/*float2 deltaTexCoord = (iTex0 - ScreenLightPos.xy);
	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;

	float3 color = txAlbedo.Sample(samClampLinear, iTex0.xy);
	half illuminationDecay = 1.0f;
	
	for (int i = 0; i < NUM_SAMPLES; i++)
  {
    iTex0 -= deltaTexCoord;
		half3 sample = txAlbedo.Sample(samClampLinear, iTex0.xy);
		
		sample *= illuminationDecay * Weight;
    color += sample;
    illuminationDecay *= Decay;
  }
  // Output final color with a further scale control factor.
  return float4( color * Exposure, 1);
	 

  // Store initial sample.
  // Set up illumination decay factor.
  // Evaluate summation from Equation 3 NUM_SAMPLES iterations.
   for (int i = 0; i < NUM_SAMPLES; i++)
  {
    // Step sample location along ray.
    texCoord -= deltaTexCoord;
    // Retrieve sample at new location.
   half3 sample = tex2D(frameSampler, texCoord);
    // Apply sample attenuation scale/decay factors.
    sample *= illuminationDecay * Weight;
    // Accumulate combined color.
    color += sample;
    // Update exponential decay factor.
    illuminationDecay *= Decay;
  }
  // Output final color with a further scale control factor.
   return float4( color * Exposure, 1);
	 */
	return float4(1,0,0,1);
}

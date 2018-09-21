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

float3 ComputeThreshold(float3 color, float thresh)
{
	return max(color - float3(thresh,thresh,thresh), float3(0,0,0));
}

float4 ComputeColorCA(float2 iTex0)
{
	// Move this as cte parameters
	float down_intensity = 0.003;
	float down_sampling = 2;
	
	float2 offset = normalize(float2(0.5, 0.5) - iTex0) * down_intensity;
	float r = txEmissive.Sample(samClampLinear, iTex0 + offset, down_sampling).r;
	float g = txEmissive.Sample(samClampLinear, iTex0, down_sampling).g;
	float b = txEmissive.Sample(samClampLinear, iTex0 - offset, down_sampling).b;
		
	return float4(r, g, b, 1);
}

float4 ComputeGhosts(float2 iTex0) 
{
	// Move this as cte parameters
	int ghost_number = 4; // number of ghost samples
	float ghost_dispersion = .3; // dispersion factor
	
	float3 result = float3(0, 0, 0);
	float2 vghost = (float2(0.5, 0.5) - iTex0) * ghost_dispersion;
	
	for (int i = 0; i < ghost_number; ++i) { 
	
		float2 offset = frac(iTex0 + vghost * float(i));
		float d = distance(offset, float2(0.5, 0.5));
		float weight = 1.0 - smoothstep(0.0, 0.75, d); 
		float3 s = ComputeColorCA(offset).xyz;
		s = ComputeThreshold(s, .2).xyz;
		result += s * weight;
	}
	
	return float4(result.xyz, 1);
}

float4 ComputeHalo(float2 iTex0)
{
	// Move this as cte parameters
	float halo_radius = 0.25;
	float halo_thickness = 0.2;

	float2 vhalo = float2(0.5, 0.5) - iTex0;
	vhalo.x /= camera_aspect_ratio;
	vhalo = normalize(vhalo);
	vhalo.x *= camera_aspect_ratio;
	
	float2 c_offset = float2(0.5, 0.0);
	float2 shift_uv = (iTex0 - c_offset) / float2(camera_aspect_ratio, 1.0) + c_offset;
	float d = distance(shift_uv, float2(0.5, 0.5));
	
	vhalo *= halo_radius;
	float halo_weight = window_cubic(d, halo_radius, halo_thickness); 
	
	float4 light_beam = ComputeColorCA(iTex0 + vhalo);
	float3 result = ComputeThreshold(light_beam.xyz, .2) * halo_weight;
	return float4(result.xyz, 1);
}

float4 ComputeStarbust(float2 iTex0)
{
	float2 vcenter = iTex0 - float2(0.5, 0.5);
	float d = length(vcenter);
	float radial = acos(vcenter.x / d);
	float star_offset = dot(camera_left, float3(0,0,1)) + dot(camera_front, float3(0,1,0));
	
	float mask = txNoiseMap2.Sample(samClampLinear, float2(radial + star_offset * 1.0, 0.0)).r
							* txNoiseMap2.Sample(samClampLinear, float2(radial - star_offset * 0.5, 0.0)).r;
		
	return saturate(mask + (1.0 - smoothstep(0.0, 0.3, d)));
}

float4 PS_PostFX_Flares(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	float2 n_iTex0 = float2(1,1) - iTex0;
	float4 color = txAlbedo.Sample(samClampLinear, iTex0);
	
	float4 lens_dirt = txNoiseMap.Sample(samClampLinear, iTex0);
	lens_dirt += ComputeStarbust(iTex0);
	
	float4 result = ComputeGhosts(n_iTex0);
	result += ComputeHalo(n_iTex0);
	
	return float4(result.xyz, 1) * lens_dirt;
}

float4 PS_PostFX_Vignette(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	float4 color = txAlbedo.Sample(samClampLinear, iTex0); 
	//float4 grade_color = txLUT.Sample(samClampLinear, color.xyz);
	float2 position = (iPosition.xy * camera_inv_resolution) - float2(0.5f,0.5f);
	//color = lerp( color, grade_color, global_shared_fx_amount );
	
	float len = length(position);
	float vignette = smoothstep(0.95, postfx_vignette - (1 - player_health) , len);
	color.rgb = lerp(color.rgb, color.rgb * vignette, 0.95);
	
	float4 greyscale = (color.r + color.g + color.b) * .333;
	color = lerp( color, greyscale, 1 - player_health);
	
	return color;
}

/*
float2 shiftChannel(float2 iTex0, float value, float shift) 
{
	float r2 = (iTex0.x - postfx_cs_offset) * (iTex0.x - postfx_cs_offset) + (iTex0.y - postfx_cs_offset) * (iTex0.y - postfx_cs_offset);
	float f = 1 + r2 * (value + shift * sqrt(r2));

	float x = f*(iTex0.x - postfx_cs_offset) + postfx_cs_offset;
	float y = f*(iTex0.y - postfx_cs_offset) + postfx_cs_offset;

	return float2(x, y);
}
*/

// PostFX Chromatic Aberration
float4 PS_PostFX_CA(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{	
	float3 glitch = txNoiseMap2.Sample(samClampPoint, iTex0).xyz;	

	float r = nrand(glitch.r, postfx_block_random * 0.025);
	float block_scan = max(0.0, ceil(postfx_scan_amount * postfx_block_amount - r));
	float2 shift_uv = (glitch.yz * 2.0 - 1.0) * block_scan;

	iTex0 = frac(iTex0 + shift_uv);
	float jitter = nrand(iTex0.y, global_world_time/20) * .2 - .1;
	jitter *= step(postfx_scan_jitter.y, abs(jitter)) * postfx_scan_jitter.x;

	//float jump = lerp(iTex0.y, frac(iTex0.y + _VerticalJump.y), _VerticalJump.x);
	//float scan_shake = (nrand(global_world_time.x, 2) - 0.5) * _HorizontalShake;
	float scan_drift = 0.25 * sin(iTex0.y + postfx_scan_drift.y * .035) * postfx_scan_drift.x;

	float4 src1 = txAlbedo.Sample(samClampLinear, frac(float2(iTex0.x + jitter* postfx_scan_amount, iTex0.y)));
	float4 src2 = txAlbedo.Sample(samClampLinear, frac(float2(iTex0.x + (jitter + scan_drift) * postfx_scan_amount, iTex0.y)));

	return float4(src1.x, src2.y, src1.z, 1);
	
	// Old deprecated chromatic aberration
/*
	float distortion = txNoiseMap.Sample( samLinear, iTex0).r;  
		
	//float2 bDist = shiftChannel(iTex0, -postfx_ca_offset, postfx_ca_offset);
	//float2 gDist = shiftChannel(iTex0, -postfx_ca_offset - postfx_ca_amount, postfx_ca_offset + postfx_ca_amount);
	//float2 rDist = shiftChannel(iTex0, -postfx_ca_offset - postfx_ca_amount * 2, postfx_ca_offset + postfx_ca_amount * 2);
  
	float4 distorsion_r = txAlbedo.Sample(samClampLinear, iTex0 + postfx_ca_amount * 0.01 * distortion);
	float4 distorsion_g = txAlbedo.Sample(samClampLinear, iTex0 - postfx_ca_amount * 0.01 * distortion);
	float4 distorsion_b = txAlbedo.Sample(samClampLinear, iTex0 + postfx_ca_amount * 0.01 * distortion);
	
	return float4(distorsion_r.r, distorsion_g.g, distorsion_b.b, 1);
	*/
}

void VS_PostFX_Focus(in float4 iPos : POSITION, out float4 oPos : SV_POSITION, out float2 oTex0 : TEXCOORD0)
{
  // Passthrough of coords and UV's
  oPos = float4(iPos.x * 2 - 1., 1 - iPos.y * 2, 0, 1);
  oTex0 = iPos.xy;
}

float4 PS_PostFX_Focus(in float4 iPosition : SV_Position, in float2 iTex0 : TEXCOORD0) : SV_Target
{
  float4 in_focus  = txAlbedo.Sample(samClampLinear, iTex0.xy);
  float4 out_focus  = txAlbedo1.Sample(samClampLinear, iTex0.xy);
  float  zlinear = txGBufferLinearDepth.Load(uint3(iPosition.xy, 0)).x * camera_zfar;

  // if focus_z_center_in_focus   = 300;
  // if focus_z_margin_in_focus   =  50;
  // if focus_transition_distance = 100;

  // We want for z between 250 and 350 => all_in_focus     ++++++++++
  // We want for z between 350 and 450 => mix between in_focus and out_Focus   XXXXX
  // We want for z between 150 and 250 => mix between in_focus and out_Focus   XXXXX
  // We want for z beyond  450 or <150 => all out_Focus    ----------
  //                        300
  // ---------XXXXXXXXXX+++++F+++++XXXXXXXXXX-------------
	
  float distance_to_focus = abs( zlinear - focus_z_center_in_focus );
  float amount_of_out_blur = smoothstep( focus_z_margin_in_focus, focus_z_margin_in_focus + focus_transition_distance, distance_to_focus );
  amount_of_out_blur = pow( amount_of_out_blur, focus_modifier);
  
  //return amount_of_out_blur;
  return amount_of_out_blur * out_focus + ( 1. - amount_of_out_blur) * in_focus;
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

#include "common.fx"

float3 gammaCorrect( float3 linear_color ) 
{
  return pow( abs(linear_color), 1. / 2.2 ); 
}

float CalculateLuminance(float3 color)
{
  return max(dot(color, float3(0.2126, 0.7152, 0.0722)), 0.05f);
}

// Retrieves the log-average lumanaince from the texture
float CalculateAvgLuminance(float2 iTex0)
{
	// This is what we should do on a separate pass
  //float3 color = txAccLights.Load(ss_load_coords).xyz;
	//float luminance = log(max(CalculateLuminance(color), 0.00001f));
	
  return txLuminance.Sample(samLinear, iTex0).x;
}

float3 CalculateExposedColor(float3 color, float avg_luminance, float thresh, float exposure)
{
	float t_luminance = max(avg_luminance, 0.001f);
	float linear_exposure = (global_exposure_adjustment / t_luminance);
	float t_exposure = log2(max(linear_exposure, 0.0001f));
	t_exposure -= thresh;
	
	return exp2(t_exposure) * color;
}

// Applies the filmic curve from John Hable's presentation
float3 toneMapFilmicALU(float3 color)
{
  color = max(0, color - 0.004f);
  color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f)+ 0.06f);

  return color;
}

float3 toneMappingReinhard(float3 hdr, float k = 1.0)
{
  return hdr / (hdr + k);
}

// PostFX Exponential distance fog.
float4 environment_fog(float4 iPosition, float2 iTex0, float3 in_color)
{
	float depth = txGBufferLinearDepth.Load(uint3(iPosition.xy, 0)).x;
	float3 wPos = getWorldCoords(iPosition.xy, depth);
	
	float3 frag_dir = (wPos - camera_pos.xyz);
	float dist = abs(length(frag_dir));
	
	float fog_factor = 1 - exp( (dist * -global_fog_density * .075)* (dist* global_fog_density * .075));	
	if(depth > 0.92) fog_factor = 0.2;

	float3 color = lerp(in_color, global_fog_env_color, saturate(fog_factor));
	//float3 color = in_color + global_fog_env_color * fog_factor;
	
	return float4(color,1);
}

// PostFX Exponential ground distance fog.
float4 ground_fog(float4 iPosition, float2 iTex0, float3 in_color)
{
	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float depth = txGBufferLinearDepth.Load(ss_load_coords).x;
	float3 wPos = getWorldCoords(iPosition.xy, depth);

	float3 frag_dir = (wPos - camera_pos.xyz);
	float dist = abs(length(frag_dir));

	//You have to tweak this values
	float be = 0.045 * smoothstep(0.0, 2.0, 60.0 - wPos.y);
	float bi = 0.075* smoothstep(0.0, 80, 10.0 - wPos.y);
	
	float fog_factor = exp(-dist * be) * (1 - exp(-dist * bi)) * global_fog_ground_density;
	float3 color = in_color * ( 1 - fog_factor) + global_fog_color * fog_factor;
		
	return float4(color,1);
}

float3 Uncharted2Tonemap(float3 x)
{
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float3 toneMappingUncharted2(float3 x) {
  float ExposureBias = 2.0f;
  float3 curr = Uncharted2Tonemap(ExposureBias*x);
  float W = 11.2;
  float3 whiteScale = 1.0f/Uncharted2Tonemap(W);
  float3 color = curr*whiteScale;
  return color;
}

// ----------------------------------------
float4 PS_Luminance(
  in float4 iPosition : SV_Position        // Screen coord, 0...800, 0..600
, in float2 iUV : TEXCOORD0
  ) : SV_Target
{
  int3 ss_load_coords = uint3(iPosition.xy, 0);
	
	// calculate the luminance using a weighted average
	float3 color = txAccLights.Load(ss_load_coords).xyz;
  float luminance = log(max(CalculateLuminance(color), 0.00001f));
  return float4(luminance, 1.0f, 1.0f, 1.0f);
}

// ----------------------------------------
float4 compute(float4 iPosition, float2 iUV)
{
  int3 ss_load_coords = uint3(iPosition.xy, 0);
  float4 oAlbedo = txGBufferAlbedos.Load(ss_load_coords);
  //return txAO.Sample(samLinear, iUV);
	//return txSelfIllum.Load(uint3(iPosition.xy,0)); // temp 
	//return txSelfIllum.Load(uint3(iPosition.xy,0)).a;
  float4 N_rt = txGBufferNormals.Load(ss_load_coords);
  float4 oNormal = float4(decodeNormal( N_rt.xyz ), 1);

  float3 hdrColor = txAccLights.Load(ss_load_coords).xyz;
	//hdrColor *= txSelfIllum.Load(uint3(iPosition.xy,0)).a;
	hdrColor = environment_fog(iPosition, iUV, hdrColor);
	hdrColor = ground_fog(iPosition, iUV, hdrColor);
		
  //hdrColor *= PS_PostFXFog(iPosition, iUV);
	//hdrColor *= txAO.Sample(samLinear, iUV);
	hdrColor *= global_exposure_adjustment;
	//hdrColor *= txAO.Sample(samLinear, iUV);	
  
	// In Low Dynamic Range we could not go beyond the value 1
  //float3 ldrColor = min(hdrColor,float3(1,1,1));
  //hdrColor = lerp( ldrColor, hdrColor, global_hdr_enabled  );

	// Preparing the eye adaption
	//float3 prev_lum = txLuminance.Load(ss_load_coords).xyz;
	//hdrColor = prev_lum + (hdrColor - prev_lum) * (1 - exp(-0.25 * 1.5));
	
	//float pixelLuminance = CalculateLuminance(hdrColor);
  //hdrColor = CalculateExposedColor(hdrColor, avg_lum, 0, 0);	
	//float3 tmColor = toneMapFilmicALU(hdrColor);
	
	// Somet one mapping.
  float3 tmColorUC2 = toneMappingUncharted2( hdrColor );
  float3 tmColorReinhard = toneMappingReinhard( hdrColor );
  float3 tmColor = lerp( tmColorReinhard, tmColorUC2, global_tone_mapping_mode );
	
  float3 gammaCorrectedColor = gammaCorrect( tmColor );
  gammaCorrectedColor = lerp( tmColor, gammaCorrectedColor.xyz, global_gamma_correction_enabled );
	
  if( global_render_output == RO_COMPLETE )
    return float4( gammaCorrectedColor, 1);
  else if( global_render_output == RO_ALBEDO )
    return float4( oAlbedo.rgb, 1 );  
  else if( global_render_output == RO_NORMAL )
    return float4( normalize( oNormal.rgb ), 1 ); 
  else if( global_render_output == RO_ROUGHNESS )
    return float4( N_rt.aaa, 1 );
  else if( global_render_output == RO_METALLIC )
    return float4( oAlbedo.aaa, 1 );
  else if( global_render_output == RO_WORLD_POS ) {
    float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;
    float3 wPos = getWorldCoords(iPosition.xy, zlinear);
    return abs(wPos.x - (int)wPos.x) * abs(wPos.z - (int)wPos.z);
  }
  else if( global_render_output == RO_DEPTH_LINEAR ) {
    float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;
    return float4(zlinear, zlinear, zlinear,1 );
  }
  else if( global_render_output == RO_AO ) {
    return txAO.Sample(samLinear, iUV);
  }
  
  return float4( gammaCorrectedColor, 1);
}

// ----------------------------------------
float4 PS_face(
  in float4 iPosition : SV_Position        // Screen coord, 0...800, 0..600
, in float2 iUV : TEXCOORD0
  ) : SV_Target
{

  float4 swapPosition = iPosition;
  swapPosition.x = 511 - swapPosition.x;

  return (compute( swapPosition, iUV ));
}

// ----------------------------------------
float4 PS(
  in float4 iPosition : SV_Position        // Screen coord, 0...800, 0..600
, in float2 iUV : TEXCOORD0
  ) : SV_Target
{
  return (compute( iPosition, iUV ));
}


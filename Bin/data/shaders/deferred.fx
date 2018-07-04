#include "common.fx"

float3 toneMappingReinhard(float3 hdr, float k = 1.0)
{
    return hdr / (hdr + k);
}

float3 gammaCorrect( float3 linear_color ) {
  return pow( abs(linear_color), 1. / 2.2 ); 
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
float4 PS(
  in float4 iPosition : SV_Position        // Screen coord, 0...800, 0..600
, in float2 iUV : TEXCOORD0
  ) : SV_Target
{
  int3 ss_load_coords = uint3(iPosition.xy, 0);
  float4 oAlbedo = txGBufferAlbedos.Load(ss_load_coords);

  float4 N_rt = txGBufferNormals.Load(ss_load_coords);
  float4 oNormal = float4(decodeNormal( N_rt.xyz ), 1);

  float3 hdrColor = txAccLights.Load(ss_load_coords).xyz;

  hdrColor *= global_exposure_adjustment;

  // In Low Dynamic Range we could not go beyond the value 1
  float3 ldrColor = min(hdrColor,float3(1,1,1));
  hdrColor = lerp( ldrColor, hdrColor, global_hdr_enabled  );

  float3 tmColorUC2 = toneMappingUncharted2( hdrColor );
  float3 tmColorReinhard = toneMappingReinhard( hdrColor );
  float3 tmColor = lerp( tmColorReinhard, tmColorUC2, global_tone_mapping_mode );

  float3 gammaCorrectedColor = gammaCorrect( tmColor );
  gammaCorrectedColor = lerp( tmColor, gammaCorrectedColor.xyz, global_gamma_correction_enabled );

	/*
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
    // Debug generated world coords
    // Recuperar la posicion de mundo para ese pixel
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
  }*/
  
  return float4( gammaCorrectedColor, 1);;
}

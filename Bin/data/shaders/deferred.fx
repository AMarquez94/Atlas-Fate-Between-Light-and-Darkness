#include "common.fx"

// ----------------------------------------
float4 PS(in float4 iPosition : SV_Position) : SV_Target
{
  int3 ss_load_coords = uint3(iPosition.xy, 0);
  float4 oAlbedo = txGBufferAlbedos.Load(ss_load_coords);
  float4 oNormal = float4(decodeNormal( txGBufferNormals.Load(ss_load_coords).xyz ), 1);

  float4 oAccLight = txAccLights.Load(ss_load_coords);
  return oAccLight; 

  //float energy = oAccLight.x + oAccLight.y + oAccLight.z;
  //if( oAccLight.x > 0.5 || oAccLight.y > 1 || oAccLight.z > 1)
  //  return float4( 0,0,0,0);

  // Debug generated world coords
  // Recuperar la posicion de mundo para ese pixel
  float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;
  float3 wPos = getWorldCoords(iPosition.xy, zlinear);
  return abs(wPos.x - (int)wPos.x) * abs(wPos.z - (int)wPos.z);

  //float4 LightsAmount = txAccLights.Sample(samLinear, input.UV);
  return oAlbedo;
}

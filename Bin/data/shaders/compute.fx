#include "common.fx"

// --------------------------------------------------------------
[numthreads(8, 8, 1)]
void kernel_bw(
  Texture2D<float> inTexture : register(t0),
  RWTexture2D<unorm float4> outTexture : register(u0),
  uint2 gid : SV_DispatchThreadID
)
{
  float4 inColor = inTexture[gid];
  float value = dot(inColor.rgb, float3(0.333, 0.333, 0.333));
  float4 outColor = float4(gid.x, gid.y, value, 1);
  outTexture[gid] = value;
}

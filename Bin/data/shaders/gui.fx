#include "common.fx"

/// --------------------
struct VS_FULL_OUTPUT {
  float4 Pos   : SV_POSITION;
  float2 UV    : TEXCOORD0;
};

float ComputePiAngle(float2 v1, float2 v2) {

	float dot = v1.x*v2.x + v1.y*v2.y;
	float det = v1.x*v2.y - v1.y*v2.x;
	return atan2(det, dot);
}

// ----------------------------------------
VS_FULL_OUTPUT VS_GUI(
  float4 iPos   : POSITION,     // 0..1, 0..1, 0 en la z
  float4 iColor : COLOR0
  )
{
  VS_FULL_OUTPUT output = (VS_FULL_OUTPUT)0;
  float4 world_pos = mul(iPos, obj_world);
  output.Pos = mul(world_pos, camera_view_proj);
  output.UV  = iPos.xy;
  return output;
}

// ----------------------------------------
float4 PS_GUI(
  VS_FULL_OUTPUT input
  ) : SV_Target
{
  float2 finalUV = lerp(minUV, maxUV, input.UV);
  float4 oDiffuse = txAlbedo.Sample(samLinear, finalUV);
  float4 oColor = float4(oDiffuse.rgb * tint_color.rgb, oDiffuse.a * tint_color.a);
  return oColor;
}

float4 PS_GUI_RADIAL(
  VS_FULL_OUTPUT input
  ) : SV_Target
{
	float2 dir1 = normalize(input.UV - float2(0.5,0.5));
	float angle = -1* (atan2(dir1.y, dir1.x) - atan2(-1, 0)) / (2*PI);
	angle = angle < 0 ? angle + 1: angle;
	clip(gui_var1 - angle);
		
  float4 oDiffuse = txAlbedo.Sample(samLinear, input.UV);
  float4 oColor = float4(oDiffuse.rgb * tint_color.rgb, oDiffuse.a * tint_color.a);
  return oColor;
}
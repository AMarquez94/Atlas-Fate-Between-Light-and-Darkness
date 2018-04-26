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
// Vertex Shaders
//--------------------------------------------------------------------------------------

VS_OUTPUT_BASIC vs_Basic(float4 iPos : POSITION, float3 iN : NORMAL, float2 iUV : TEXCOORD0, float4 iTan : NORMAL1)
{
	VS_OUTPUT_BASIC output = (VS_OUTPUT_BASIC)0;
	float4 world_pos = mul(iPos, obj_world);
	output.Pos = mul(world_pos, camera_view);
	output.Pos = mul(output.Pos, camera_proj);
	output.wPos = world_pos.xyz;

	output.N = mul(iN, (float3x3)obj_world);
	output.T.xyz = mul(iTan.xyz, (float3x3)obj_world);
	output.T.w = iTan.w;      // Keep the w as is
	output.UV = iUV;
	
	return output;
}

float4 ps_Basic(VS_OUTPUT_BASIC input) : SV_Target
{
	float3 wN = computeNormalMap( input.N, input.T, input.UV );

	// Convert pixel position in world space to light space
	float4 pos_in_light_proj_space = mul(float4(input.wPos,1), light_view_proj_offset);
	float3 pos_in_light_homo_space = pos_in_light_proj_space.xyz / pos_in_light_proj_space.w; // -1..1																							  // Use these coords to access the projector texture of the light dir
	float4 light_projector_color = txLightProjector.Sample(samBorderLinear, pos_in_light_homo_space.xy);

	// Fade to zero in the last 1% of the zbuffer of the light
	light_projector_color *= smoothstep(1.0f, 0.99f, pos_in_light_homo_space.z);
	
	return light_projector_color;
}

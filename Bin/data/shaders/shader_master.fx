#include "ctes.h"

Texture2D    txDiffuse      : register(t0);
Texture2D    txLightmap     : register(t1);
Texture2D    txLightProjector : register(t5);  // TS_LIGHT_PROJECTOR

SamplerState samLinear      : register(s0);
SamplerState samBorderLinear : register(s1);

static float3 Light = float3(-0.3575,0.7163,-0.5991);

// Variable declaration
  
struct VS_OUTPUT_CTCOLOR
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR0;
};

struct VS_OUTPUT_BASIC
{
	float4 Pos : SV_POSITION;
	float3 N : NORMAL;
	float2 UV : TEXCOORD0;
	float3 wPos : TEXCOORD1;
};

struct VS_OUTPUT_LIGHTMAP
{
	float4 Pos : SV_POSITION;
	float3 wPos : TEXCOORD2;
	float3 N : NORMAL;
	float2 UV : TEXCOORD0;
	float2 UVB : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Vertex Shaders
//--------------------------------------------------------------------------------------

VS_OUTPUT_CTCOLOR vs_CtColor(float4 Pos : POSITION, float4 Color : COLOR)
{
	VS_OUTPUT_CTCOLOR output = (VS_OUTPUT_CTCOLOR)0;
	output.Pos = mul(Pos, obj_world);
	output.Pos = mul(output.Pos, camera_view);
	output.Pos = mul(output.Pos, camera_proj);
	output.Color = Color;
	return output;
}

VS_OUTPUT_BASIC vs_Basic(float4 Pos : POSITION, float3 N : NORMAL, float2 UV : TEXCOORD0)
{
	VS_OUTPUT_BASIC output = (VS_OUTPUT_BASIC)0;
	float4 world_pos = mul(Pos, obj_world);
	output.Pos = mul(world_pos, camera_view);
	output.Pos = mul(output.Pos, camera_proj);
	output.wPos = world_pos.xyz;

	output.N = mul(N, (float3x3)obj_world);
	output.UV = UV;
	return output;
}

VS_OUTPUT_LIGHTMAP vs_Lightmap( float4 Pos : POSITION, float3 N : NORMAL, float2 UV : TEXCOORD0 , float2 UVB : TEXCOORD1)
{
	VS_OUTPUT_LIGHTMAP output = (VS_OUTPUT_LIGHTMAP)0;
	float4 world_pos = mul(Pos, obj_world);
	output.Pos = mul(world_pos, camera_view);
	output.Pos = mul(output.Pos, camera_proj);	
	output.wPos = world_pos.xyz;

	// Rotate the normal
	output.N = mul(N, (float3x3)obj_world);
	output.UV = UV;
	output.UVB = UVB;
	
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------

float4 ps_Ctcolor(VS_OUTPUT_CTCOLOR input) : SV_Target
{
	return input.Color;
}

float4 ps_BasicNormal(VS_OUTPUT_BASIC input) : SV_Target
{
	Light = normalize( Light );
	float diffuseAmount = dot( input.N, Light );
	diffuseAmount = saturate( 0.2 + diffuseAmount );
	diffuseAmount = 0.3 + diffuseAmount * 0.7;

	return obj_color * diffuseAmount;
}

float4 ps_Basic(VS_OUTPUT_BASIC input) : SV_Target
{
	// Convert pixel position in world space to light space
	float4 pos_in_light_proj_space = mul(float4(input.wPos,1), light_view_proj_offset);
	float3 pos_in_light_homo_space = pos_in_light_proj_space.xyz / pos_in_light_proj_space.w; // -1..1																							  // Use these coords to access the projector texture of the light dir
	float4 light_projector_color = txLightProjector.Sample(samBorderLinear, pos_in_light_homo_space.xy);

	// Fade to zero in the last 1% of the zbuffer of the light
	light_projector_color *= smoothstep(1.0f, 0.99f, pos_in_light_homo_space.z);
	float3 Light = light_pos - input.wPos;
	Light = normalize(Light);
	
	float diffuseAmount = dot(input.N, Light);
	diffuseAmount = saturate(0.2 + diffuseAmount);
	diffuseAmount = 0.2 + diffuseAmount;

	float4 texture_albedo = txDiffuse.Sample(samLinear, input.UV);
	return (texture_albedo * diffuseAmount * obj_color) + (light_projector_color* light_intensity * light_color);
}

float4 ps_Lightmap(VS_OUTPUT_LIGHTMAP input) : SV_Target
{
	// Convert pixel position in world space to light space
	float4 pos_in_light_proj_space = mul(float4(input.wPos,1), light_view_proj_offset);
	float3 pos_in_light_homo_space = pos_in_light_proj_space.xyz / pos_in_light_proj_space.w; // -1..1

	// Use these coords to access the projector texture of the light dir
	float4 light_projector_color = txLightProjector.Sample(samBorderLinear, pos_in_light_homo_space.xy);
	if (pos_in_light_proj_space.z < 0.)
		light_projector_color = float4(0,0,0,0);

	// Fade to zero in the last 1% of the zbuffer of the light
	light_projector_color *= smoothstep(1.0f, 0.99f, pos_in_light_homo_space.z);

	float3 Light = light_pos - input.wPos;

	Light = normalize( Light );
	float diffuseAmount = dot( input.N, Light );
	diffuseAmount = saturate( 0.2 + diffuseAmount );
	diffuseAmount = 0.3 + diffuseAmount;

	float4 texture_albedo = txDiffuse.Sample(samLinear, input.UV);
	float4 texture_lightmap = txLightmap.Sample(samLinear, input.UVB);
	return texture_albedo * texture_lightmap * diffuseAmount * obj_color;
}
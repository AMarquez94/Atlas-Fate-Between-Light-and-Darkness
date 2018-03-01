#include "ctes.h"

Texture2D    txDiffuse      : register(t0);
Texture2D    txLightmap     : register(t1);
SamplerState samLinear      : register(s0);

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
};

struct VS_OUTPUT_LIGHTMAP
{
	float4 Pos : SV_POSITION;
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
	output.Color = obj_color;
	return output;
}

VS_OUTPUT_BASIC vs_Basic(float4 Pos : POSITION, float3 N : NORMAL, float2 UV : TEXCOORD0)
{
	VS_OUTPUT_BASIC output = (VS_OUTPUT_BASIC)0;
	output.Pos = mul(Pos, obj_world);
	output.Pos = mul(output.Pos, camera_view);
	output.Pos = mul(output.Pos, camera_proj);
	output.N = mul(N, (float3x3)obj_world);
	output.UV = UV;
	return output;
}

VS_OUTPUT_LIGHTMAP vs_Lightmap( float4 Pos : POSITION, float3 N : NORMAL, float2 UV : TEXCOORD0 , float2 UVB : TEXCOORD1)
{
	VS_OUTPUT_LIGHTMAP output = (VS_OUTPUT_LIGHTMAP)0;
	output.Pos = mul(Pos, obj_world);
	output.Pos = mul(output.Pos, camera_view);
	output.Pos = mul(output.Pos, camera_proj);
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

float4 ps_Basic(VS_OUTPUT_BASIC input) : SV_Target
{
	float3 Light = float3(1,1,1);
	Light = normalize( Light );
	float diffuseAmount = dot( input.N, Light );
	diffuseAmount = saturate( 0.2 + diffuseAmount );
	diffuseAmount = 0.3 + diffuseAmount * 0.7;

	float4 texture_color = txDiffuse.Sample(samLinear, input.UV);
	return texture_color * obj_color * diffuseAmount;
}

float4 ps_Lightmap(VS_OUTPUT_LIGHTMAP input) : SV_Target
{
	float3 Light = float3(1,1,1);
	Light = normalize( Light );
	float diffuseAmount = dot( input.N, Light );
	diffuseAmount = saturate( 0.2 + diffuseAmount );
	diffuseAmount = 0.3 + diffuseAmount * 0.7;

	float4 texture_color = txDiffuse.Sample(samLinear, input.UV);
	float4 texture_color2 = txLightmap.Sample(samLinear, input.UVB);
	return texture_color2 * texture_color * obj_color;
}
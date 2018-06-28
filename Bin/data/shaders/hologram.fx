//--------------------------------------------------------------------------------------
#include "common.fx"

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Vertex
//--------------------------------------------------------------------------------------
void VS_GBuffer_Hologram(
	in float4 iPos     : POSITION
	, in float3 iNormal : NORMAL0
	, in float2 iTex0 : TEXCOORD0
	, in float2 iTex1 : TEXCOORD1
	, in float4 iTangent : NORMAL1

	, out float4 oPos : SV_POSITION
	, out float3 oNormal : NORMAL0
	, out float4 oTangent : NORMAL1
	, out float2 oTex0 : TEXCOORD0
	, out float2 oTex1 : TEXCOORD1
	, out float3 oWorldPos : TEXCOORD2
)
{
	iPos.x += 0.5 * (step(0.5, sin(global_world_time * 2.0 + iPos.y * 1.0)) * step(0.99, sin(global_world_time *4 * 0.5)));
	
	float4 world_pos = mul(iPos, obj_world);
	oPos = mul(world_pos, camera_view_proj);
	
	// Rotar la normal segun la transform del objeto
	oNormal = mul(iNormal, (float3x3)obj_world);
	oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
	oTangent.w = iTangent.w;

	// Las uv's se pasan directamente al ps
	oTex0 = iTex0;
	oTex1 = iTex1;
	oWorldPos = world_pos.xyz;
}

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Pixel
//--------------------------------------------------------------------------------------
float4 PS_GBuffer_Hologram(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
): SV_Target0
{

	// Retrieve main colors.
	float4 albedo = txAlbedo.Sample(samLinear, iTex0);
	float4 flicker = txNoiseMap.Sample(samLinear, iTex0 );
		
	float4 rim_base_color = float4(0,1,1,1);
		
	// Compute the scanline
	float vertex_sift = (dot(iWorldPos, normalize(float4(float3(0,-1,0), 1.0))) + 1) / 2;
	float scan = step(frac(vertex_sift * 22 + global_world_time * 4), 0.5) * 0.65;
		
	// Compute the glow factor
	float glow = frac(-vertex_sift * 1 - global_world_time * 0.7);
	
	// Compute the rim factor
	float3 view_dir = normalize((iWorldPos - camera_pos).xyz);
	float rim = 1.0 - saturate(dot(-view_dir, iNormal));
	float4 rim_color = rim_base_color * pow(rim, 4.1);
	
	// Compute the final result
	float4 final_color = rim_base_color * albedo + (glow * 0.35 * albedo) + rim_color;
	final_color.a = color_material.a * ( scan + rim + glow ) * 0.35;
	return final_color;
}

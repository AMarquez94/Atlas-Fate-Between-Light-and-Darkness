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

void VS_SKIN_GBuffer_Hologram(
	float4 iPos : POSITION
	, float3 iN : NORMAL
	, float2 iUV : TEXCOORD
	, float4 iTangent : TANGENT
	, int4   iBones : BONES
	, float4 iWeights : WEIGHTS

	, out float4 oPos : SV_POSITION
	, out float3 oNormal : NORMAL0
	, out float4 oTangent : NORMAL1
	, out float2 oTex0 : TEXCOORD0
	, out float2 oTex1 : TEXCOORD1
	, out float3 oWorldPos : TEXCOORD2
)
{

	// Faking the verterx shader by now since we don't have tangents...
	float4x4 skin_mtx = getSkinMtx(iBones, iWeights);
	
	// Regular transforms
	float4 world_pos = mul(iPos, skin_mtx);
	float vertex_sift = (dot(world_pos, normalize(float4(float3(0,-1,0), 1.0))) + 1) * .5;
	float scan = frac(vertex_sift * 35 + global_world_time * 20) * 0.56;
	float scan_shift = 1 - step(frac(vertex_sift * 6 + global_world_time), 0.65);
	iPos.x += scan_shift * (nrand(-global_world_time, global_world_time) * 2 - 1)* 0.05;
	
	float4 skinned_Pos = mul(float4(iPos.xyz * BonesScale, 1), skin_mtx);
	
	oPos = mul(skinned_Pos, camera_view_proj); // Transform to viewproj, w_m inside skin_m
	oNormal = mul(iN, (float3x3)skin_mtx); // Rotate the normal
	oTangent.xyz = mul(iTangent.xyz, (float3x3)skin_mtx);
	oTangent.w = iTangent.w;

	oTex0 = iUV;
	oTex1 = iUV;
	oWorldPos = skinned_Pos.xyz;
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
	float vertex_sift = (dot(iWorldPos, normalize(float3(0,-1,0))) + 1) / 2;
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


//--------------------------------------------------------------------------------------
// Hologram: Star Wars like
//--------------------------------------------------------------------------------------
void VS_GBuffer_SWHologram(
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
	, out float3 oModelPos : TEXCOORD3
	, out float max_height : TEXCOORD4
)
{

	// Displacement control
	float prev_height = iPos.y;
	max_height = 1;//abs(sin(global_world_time));
	float int_width = 0.75;//abs(sin(global_world_time)) * 0.5;

	iPos.y *= max_height;
	float3 disp_center = float3(0, iPos.y, 0);
	float3 disp_dir = disp_center + (iPos.xyz - disp_center) * (1 * int_width + int_width * prev_height);
	iPos = float4(disp_dir,1);
	
	// Regular transforms
	float4 world_pos = mul(iPos, obj_world);
	float vertex_sift = (dot(world_pos, normalize(float4(float3(0,-1,0), 1.0))) + 1) * .5;
	float scan = frac(vertex_sift * 35 + global_world_time * 270) * 0.96;
	world_pos.xyz += iNormal * scan * 0.025;
	
	oPos = mul(world_pos, camera_view_proj);
	//oModelPos = mul(float3(0,0,0), obj_world); 
	oModelPos = float3(obj_world[3][0], obj_world[3][1], obj_world[3][2]);
	
	oNormal = mul(iNormal, (float3x3)obj_world);
	oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
	oTangent.w = iTangent.w;

	oTex0 = iTex0;
	oTex1 = iTex1;
	oWorldPos = world_pos.xyz;
}

float4 PS_GBuffer_SWHologram(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
	, float3 iModelPos : TEXCOORD3
	, float  iMaxHeight : TEXCOORD4
): SV_Target0
{
	// Compute the scanline
	float vertex_sift = (dot(iWorldPos, normalize(float3(0,-1,0))) + 1) * .5;
	float scan = frac(vertex_sift * 35 + global_world_time * 70) * 0.96;

	// Compute the glow factor
	float glow = frac(-vertex_sift * 1 - global_world_time * 0.7);
	
	float3 dir = normalize(iWorldPos.xyz - camera_pos);
	float theta = pow(dot(dir, iNormal), 2);
	
	float t_dist = distance(iModelPos, iWorldPos.xyz);
	float att = 1 - distance(iModelPos, iWorldPos.xyz) / (2*iMaxHeight + 0.75);
	float att2 = 1 / (distance(float3(iModelPos.x, iModelPos.y, iModelPos.z), iWorldPos.xyz) * 4);

	return scan * float4(0,0.15,1,1) * att * att2 * theta * 8; 
}

void VS_GBuffer_SWHologram_Model(
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
	, out float3 oModelPos : TEXCOORD3
)
{
	// Regular transforms
	float4 world_pos = mul(iPos, obj_world);
	float vertex_sift = (dot(world_pos, normalize(float4(float3(0,-1,0), 1.0))) + 1) * .5;
	float scan = frac(vertex_sift * 35 + global_world_time * 20) * 0.56;
	float scan_shift = 1 - step(frac(vertex_sift * 6 + global_world_time), 0.65);
	iPos.x += scan_shift * (nrand(-global_world_time, global_world_time) * 2 - 1)* 0.05;
	world_pos = mul(iPos, obj_world);
	
	oPos = mul(world_pos, camera_view_proj);
	//oModelPos = mul(float3(0,0,0), obj_world); 
	oModelPos = float3(obj_world[3][0], obj_world[3][1], obj_world[3][2]);
	
	oNormal = mul(iNormal, (float3x3)obj_world);
	oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
	oTangent.w = iTangent.w;

	oTex0 = iTex0;
	oTex1 = iTex1;
	oWorldPos = world_pos.xyz;
}

float4 PS_GBuffer_SWHologram_Model(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
	, float3 iModelPos : TEXCOORD3
	, float  iMaxHeight : TEXCOORD4
): SV_Target0
{
	// Compute the scanline
	float vertex_sift = (dot(iWorldPos, normalize(float3(0,-1,0))) + 1) * .5;
	float scan = frac(vertex_sift * 80) * 0.86;
	
	float3 dir = normalize(iWorldPos.xyz - camera_pos);
	float theta = abs(1- pow(dot(dir, iNormal), 2));
	
	float4 flicker = txNoiseMap.Sample(samLinear, iTex0 * global_world_time);
	
	// Compute the glow factor
	float glow = 1 - step(frac(vertex_sift * 6 + global_world_time), 0.95);
	
	return scan * float4(0,0.25,1,1) * theta + glow * 0.12 * scan; 
}

float4 PS_GBuffer_SWPlayer(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
	, float3 iModelPos : TEXCOORD3
	, float  iMaxHeight : TEXCOORD4
): SV_Target0
{
	// Compute the scanline
	float4 noise0 = txNoiseMap.Sample(samLinear, iTex0);
	float vertex_sift = (dot(iWorldPos, normalize(float3(0,-1,0))) + 1) * .35;
	float scan = frac(vertex_sift * 120) * 1.6;
	
	float3 dir = normalize(iWorldPos.xyz - camera_pos);
	float theta = abs(1- pow(dot(dir, iNormal), 2));
	
	float4 flicker = txNoiseMap.Sample(samLinear, iTex0 * global_world_time);
	
	// Compute the glow factor
	float glow = 1 - step(frac(vertex_sift * 6 + global_world_time), 0.95);	
	clip(noise0.x - self_opacity);
	float4 color = float4(0,0.25,1,1);

	if((noise0.x - self_opacity) < 0.1f){
		color = float4(1,1,1,1);
	}
	
	return scan * color * theta + glow * scan * 0.24; 
}


void VS_HologramScreen(
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

float4 PS_HologramScreen(  
	float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float2 iWorldPos : TEXCOORD
  ): SV_Target0
{
	float vertex_sift = (dot(iWorldPos, normalize(float3(0,-1,0)))  + global_world_time);
	float scan = frac(vertex_sift * 100) * 3.2;
	float4 color = float4(0,0.15,1,1);
	float4 albedo = txAlbedo.Sample(samLinear, iTex0);
	
	return albedo * color * scan * albedo.a;
}
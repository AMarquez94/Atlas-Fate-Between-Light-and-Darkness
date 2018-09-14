//--------------------------------------------------------------------------------------
#include "common.fx"

//--------------------------------------------------------------------------------------
void computeBlendWeights2( float t1_a
                        , float t2_a
                        , float t3_a
                        , out float w1
                        , out float w2 
                        , out float w3 
                        ) {
  float depth = 0.35;
  float ma = max( t1_a, max( t2_a, t3_a ) ) - depth;
  float b1 = max( t1_a - ma, 0 );
  float b2 = max( t2_a - ma, 0 );
  float b3 = max( t3_a - ma, 0 );
  float b_total = b1 + b2 + b3;
  w1 = b1 / ( b_total );
  w2 = b2 / ( b_total );
  w3 = b3 / ( b_total );
}


float2 ComputeWeightedParallax(float2 texCoords, float3 view_dir, float2 iTex1) {

	float4 weight_texture_boost = txMixBlendWeights.Sample(samLinear, iTex1);
  float albedoR = txAlbedo.Sample(samLinear, texCoords).a;
  float albedoG = txAlbedo1.Sample(samLinear, texCoords).a;
  float albedoB = txAlbedo2.Sample(samLinear, texCoords).a;
		
  float w1, w2, w3;
  computeBlendWeights2( albedoR + weight_texture_boost.r
                     , albedoG + weight_texture_boost.g
                     , albedoB + weight_texture_boost.b
                     , w1, w2, w3 );
										 
	const float minLayers = 15.0;
	const float maxLayers = 30.0;
	float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), view_dir)));
	float layerDepth = 1.0 / numLayers;
	float currentLayerDepth = 0.0;
	float2 P = view_dir.xy * 0.075;
	float2 deltaTexCoords = P / numLayers;

	float2 currentTexCoords = texCoords;
	float heightR = txHeight.Sample(samLinear, currentTexCoords).r;
	float heightG = txHeight1.Sample(samLinear, currentTexCoords).r;
	float heightB = txHeight2.Sample(samLinear, currentTexCoords).r;
			
	float height_f1 = heightR * w1 + heightG * w2 + heightB * w3;
	float currentDepthMapValue = 1 - height_f1;

	[unroll(130)]
	while (currentLayerDepth < currentDepthMapValue)
	{
		currentTexCoords -= deltaTexCoords;
		heightR = txHeight.Sample(samLinear, currentTexCoords).r;
		heightG = txHeight1.Sample(samLinear, currentTexCoords).r;
		heightB = txHeight2.Sample(samLinear, currentTexCoords).r;
		height_f1 = heightR * w1 + heightG * w2 + heightB * w3;
		
		currentDepthMapValue = 1 - height_f1;
		currentLayerDepth += layerDepth;
	}

	float2 prevTexCoords = currentTexCoords + deltaTexCoords;
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	heightR = txHeight.Sample(samLinear, prevTexCoords).r;
	heightG = txHeight1.Sample(samLinear, prevTexCoords).r;
	heightB = txHeight2.Sample(samLinear, prevTexCoords).r;
	height_f1 = heightR * w1 + heightG * w2 + heightB * w3;
	
	float beforeDepth = (1 - height_f1) - currentLayerDepth + layerDepth;
	float weight = afterDepth / (afterDepth - beforeDepth);
	float2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
	
	return finalTexCoords;
}

float2 ComputeParallax(float2 texCoords, float3 view_dir, out float p_height) {

	const float minLayers = 15.0;
	const float maxLayers = 30.0;
	float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), view_dir)));
	float layerDepth = 1.0 / numLayers;
	float currentLayerDepth = 0.0;
	float2 P = view_dir.xy * 0.095;
	float2 deltaTexCoords = P / numLayers;

	float2 currentTexCoords = texCoords;
	float currentDepthMapValue = 1 - txHeight.Sample(samLinear, currentTexCoords).r;

	[unroll(130)]
	while (currentLayerDepth < currentDepthMapValue)
	{
		currentTexCoords -= deltaTexCoords;
		currentDepthMapValue = 1 - txHeight.Sample(samLinear, currentTexCoords).r;
		currentLayerDepth += layerDepth;
	}

	float2 prevTexCoords = currentTexCoords + deltaTexCoords;
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = (1 - txHeight.Sample(samLinear, prevTexCoords).r) - currentLayerDepth + layerDepth;
	float weight = afterDepth / (afterDepth - beforeDepth);
	float2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	p_height = currentLayerDepth + beforeDepth * weight + afterDepth * (1.0 - weight);
	
	return finalTexCoords;
}

float ComputeParallaxShadow(float2 texCoords, float3 light_dir, float init_height)
{
	float shadowMultiplier = 1;
	
	if(dot(float3(0,0,1), light_dir) > 0){
	
		const float minLayers = 15.0;
		const float maxLayers = 30.0;
		float numSamplesUnderSurface = 0;
		float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), light_dir)));
		float layerHeight	= init_height / numLayers;
		float2 texStep = 0.075 * light_dir.xy / numLayers;

		float currentLayerHeight	= init_height - layerHeight;
		float2 currentTextureCoords	= texCoords + texStep;
		float heightFromTexture	= 1 - txHeight.Sample(samLinear, currentTextureCoords).r;
		int stepIndex	= 1;

		[unroll(130)]
		while(currentLayerHeight > 0)
		{
			 // if point is under the surface
			 if(heightFromTexture < currentLayerHeight)
			 {
					numSamplesUnderSurface	+= 1;
					float newShadowMultiplier	= (currentLayerHeight - heightFromTexture) * (1.0 - stepIndex / numLayers);
					shadowMultiplier	= max(shadowMultiplier, newShadowMultiplier);
			 }
			 
			 stepIndex	+= 1;
			 currentLayerHeight	-= layerHeight;
			 currentTextureCoords	+= texStep;
			 heightFromTexture	= 1 - txHeight.Sample(samLinear, currentTextureCoords).r;
		}

		if(numSamplesUnderSurface < 1)
		{
			 shadowMultiplier = 1;
		}
		else
		{
			 shadowMultiplier = 1.0 - shadowMultiplier;
		}
	}
	
	return shadowMultiplier;
}

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Vertex
//--------------------------------------------------------------------------------------
void VS_GBuffer(
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
	, out float3 oTanView : TEXCOORD3
	, out float3 oTanLight : TEXCOORD4
	, out float3 oTanFrag : TEXCOORD5
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

	float3x3 TBN = transpose(computeTBN(oNormal, oTangent));
	oTanView = mul(camera_pos, TBN);
	oTanLight = mul(light_pos, TBN);
	oTanFrag = mul(oWorldPos, TBN);
}

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Pixel
//--------------------------------------------------------------------------------------
void PS_GBuffer_Parallax(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
  , float3 iTanView : TEXCOORD3
	, float3 iTanLight : TEXCOORD4
  , float3 iTanFrag : TEXCOORD5
	
  , out float4 o_albedo : SV_Target0
  , out float4 o_normal : SV_Target1
  , out float1 o_depth : SV_Target2
  , out float4 o_selfIllum : SV_Target3
)
{
	// Shift the uv's by the parallax effect
	float3 view_dir = normalize(iTanView - iTanFrag);
	float3 view_light = normalize(iTanLight - iTanFrag);
	
	float p_height = 0;
	iTex0 = ComputeParallax(iTex0, view_dir, p_height);
  //float shadowMultiplier = ComputeParallaxShadow(iTex0, view_light, p_height - 0.05);
	
	o_albedo = txAlbedo.Sample(samLinear, iTex0);
	o_albedo.a = txMetallic.Sample(samLinear, iTex0).r;
	o_selfIllum = txEmissive.Sample(samLinear, iTex0) * self_intensity;
	o_selfIllum.xyz *= self_color.xyz;
	o_selfIllum.a = txAOcclusion.Sample(samLinear, iTex0).r;
	
	float roughness = txRoughness.Sample(samLinear, iTex0).r;
	float3 N = computeNormalMap(iNormal, iTangent, iTex0);
	o_normal = encodeNormal(N, roughness);

	if (scalar_metallic >= 0.f)
	o_albedo.a = scalar_metallic;
	if (scalar_roughness >= 0.f)
	o_normal.a = scalar_roughness;

	float3 camera2wpos = iWorldPos - camera_pos;
	o_depth = dot(camera_front.xyz, camera2wpos) / camera_zfar;
}


//--------------------------------------------------------------------------------------
void PS_GBufferMix(
  float4 Pos : SV_POSITION
, float3 iNormal : NORMAL0
, float4 iTangent : NORMAL1
, float2 iTex0 : TEXCOORD0
, float2 iTex1 : TEXCOORD1
, float3 iWorldPos : TEXCOORD2
, float3 iTanView : TEXCOORD3
, float3 iTanLight : TEXCOORD4
, float3 iTanFrag : TEXCOORD5
, out float4 o_albedo : SV_Target0
, out float4 o_normal : SV_Target1
, out float1 o_depth : SV_Target2
, out float4 o_selfIllum : SV_Target3
)
{

	// Shift the uv's by the parallax effect
	float3 view_dir = normalize(iTanView - iTanFrag);
	float3 view_light = normalize(iTanLight - iTanFrag);
	
	float p_height = 0;
	//iTex0 = ComputeParallax(iTex0, view_dir, p_height);
	iTex0 = ComputeWeightedParallax(iTex0, view_dir, iTex1);
	
  // This is different -----------------------------------------
	float4 weight_texture_boost = txMixBlendWeights.Sample(samLinear, iTex1);
  float4 albedoR = txAlbedo.Sample(samLinear, iTex0);
  float4 albedoG = txAlbedo1.Sample(samLinear, iTex0);
  float4 albedoB = txAlbedo2.Sample(samLinear, iTex0);
		
  float w1, w2, w3;
  computeBlendWeights2( albedoR.a + weight_texture_boost.r
                     , albedoG.a + weight_texture_boost.g
                     , albedoB.a + weight_texture_boost.b
                     , w1, w2, w3 );
  
	// Use the weight to 'blend' the albedo colors
  float4 albedo = albedoR * w1 + albedoG * w2 + albedoB * w3;
  o_albedo.xyz = albedo.xyz;
  o_selfIllum.xyz *= self_color.xyz;
  //float aoR = txAOcclusion.Sample(samLinear, iTex0).r;
  //float aoG = txAOcclusion1.Sample(samLinear, iTex0).r;
  //float aoB = txAOcclusion2.Sample(samLinear, iTex0).r;
  //float ao_color = aoR * w1 + aoG * w2 + aoB * w3; 
  o_selfIllum = float4(0,0,0,1);//ao_color;
	
  // This isMix the normal
  float3 normalR = txNormal.Sample(samLinear, iTex0).xyz * 2.0 - 1.0;
  float3 normalG = txNormal1.Sample(samLinear, iTex0).xyz * 2.0 - 1.0;
  float3 normalB = txNormal2.Sample(samLinear, iTex0).xyz * 2.0 - 1.0;
  float3 normal_color = normalR * w1 + normalG * w2 + normalB * w3; 
  float3x3 TBN = computeTBN( iNormal, iTangent );

  // Normal map comes in the range 0..1. Recover it in the range -1..1
  float3 wN = mul( normal_color, TBN );
  float3 N = normalize( wN );

  float metallicR = txMetallic.Sample(samLinear, iTex0).r;
	float metallicG = txMetallic1.Sample(samLinear, iTex0).r;
	float metallicB = txMetallic2.Sample(samLinear, iTex0).r;
  float metallic_color = metallicR * w1 + metallicG * w2 + metallicB * w3; 
  o_albedo.a = metallic_color;

  // Save roughness in the alpha coord of the N render target
  float roughnessR = txRoughness.Sample(samLinear, iTex0).r;
	float roughnessG = txRoughness1.Sample(samLinear, iTex0).r;
	float roughnessB = txRoughness2.Sample(samLinear, iTex0).r;
  float roughness_color = roughnessR * w1 + roughnessG * w2 + roughnessB * w3; 
	
  o_normal = encodeNormal( N, roughness_color );

  // Compute the Z in linear space, and normalize it in the range 0...1
  // In the range z=0 to z=zFar of the camera (not zNear)
  float3 camera2wpos = iWorldPos - camera_pos;
  o_depth = dot( camera_front.xyz, camera2wpos ) / camera_zfar;
}

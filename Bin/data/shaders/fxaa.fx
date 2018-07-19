#include "postfx.fx"

float4 PS_FXAA(float4 position : SV_POSITION,float2 texcoord : TEXCOORD0) : SV_TARGET
{			
	float FXAA_SPAN_MAX = 8.0;
	float FXAA_REDUCE_MUL = 1.0/64.0;
	float FXAA_REDUCE_MIN = 1.0/128.0;
	float2 frameBufSize = 1 / camera_inv_resolution;
	
	float3 rgbNW = txAlbedo.Sample(samClampLinear,texcoord+(float2(-1.0,-1.0)/frameBufSize)).xyz;
	float3 rgbNE = txAlbedo.Sample(samClampLinear,texcoord+(float2(1.0,-1.0)/frameBufSize)).xyz;
	float3 rgbSW = txAlbedo.Sample(samClampLinear,texcoord+(float2(-1.0,1.0)/frameBufSize)).xyz;
	float3 rgbSE = txAlbedo.Sample(samClampLinear,texcoord+(float2(1.0,1.0)/frameBufSize)).xyz;
	float3 rgbM = txAlbedo.Sample(samClampLinear,texcoord).xyz;
	float3 luma = float3(0.299, 0.587, 0.114);
	
	float lumaNW = dot(rgbNW, luma);
	float lumaNE = dot(rgbNE, luma);
	float lumaSW = dot(rgbSW, luma);
	float lumaSE = dot(rgbSE, luma);
	float lumaM  = dot(rgbM,  luma);

	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	float2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

	float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

	dir = min(float2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
				max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
				dir * rcpDirMin)) / frameBufSize;

	float3 rgbA = (1.0/2.0) * (
			txAlbedo.Sample(samClampLinear, texcoord.xy + dir * (1.0/3.0 - 0.5)).xyz +
			txAlbedo.Sample(samClampLinear, texcoord.xy + dir * (2.0/3.0 - 0.5)).xyz);
	float3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
			txAlbedo.Sample(samClampLinear, texcoord.xy + dir * (0.0/3.0 - 0.5)).xyz +
			txAlbedo.Sample(samClampLinear, texcoord.xy + dir * (3.0/3.0 - 0.5)).xyz);
	float lumaB = dot(rgbB, luma);

	float4 final_color;
	if((lumaB < lumaMin) || (lumaB > lumaMax)){
		final_color = float4(rgbA,1);
	}else{
		final_color = float4(rgbB,1);
	}
	//return final_color;
	
	//return postfx_vignette(position, texcoord);
	return postfx_contrast(final_color);
}
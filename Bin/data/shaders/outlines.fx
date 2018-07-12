//--------------------------------------------------------------------------------------
#include "common.fx"

// https://stackoverflow.com/questions/34601325/directx11-read-stencil-bit-from-compute-shader
Texture2D<uint2>    txBackBufferStencil        SLOT(TS_ALBEDO);


// In case we want to use the stencil test, we must have the zbuffer+stencil from the backbuffer
// actived when rendering the outlines pass
// But we can't read the current values as it's 
// In the tech 'outlines' add this ps entry point instead of the default PS
//    "ps_entry_point": "PS_Solid",

float4 PS_Solid() : SV_Target
{
    return float4(0,0.5,0,0.5);
}

//--------------------------------------------------------------------------------------
float4 PS(float4 iPosition : SV_POSITION, float2 UV : TEXCOORD0) : SV_Target
{    

/*
	float3 shockParams = float3(10, 0.8, 0.1);
  float dist = distance(UV, float2(0,0));
	
  if ( (dist <= (linear_time + shockParams.z)) && 
       (dist >= (linear_time - shockParams.z)) ) 
  {
    float diff = (dist - linear_time); 
    float powDiff = 1.0 - pow(abs(diff*shockParams.x), 
                                shockParams.y); 
    float diffTime = diff  * powDiff; 
    float2 diffUV = normalize(UV - float2(0,0)); 
    iPosition = float4(UV + (diffUV * diffTime), iPosition.z, iPosition.w);
  } 
*/	
    // Retrieve the linear depth on given pixel
    int3 ss_load_coords = uint3(iPosition.xy, 0);
    float depth = txGBufferLinearDepth.Load(ss_load_coords).x;

    float edge = 0;
    float _EDGE = 2.0f;
    float _PULSE = 0.1f;

	float average = 0.125f * (
		  txGBufferLinearDepth.Load(ss_load_coords + int3(1,-1,0)).x
		+ txGBufferLinearDepth.Load(ss_load_coords + int3(0,-1,0)).x
		+ txGBufferLinearDepth.Load(ss_load_coords + int3(-1,-1,0)).x
		+ txGBufferLinearDepth.Load(ss_load_coords + int3(1,0,0)).x	
		+ txGBufferLinearDepth.Load(ss_load_coords + int3(-1,0,0)).x
		+ txGBufferLinearDepth.Load(ss_load_coords + int3(1, 1,0)).x
		+ txGBufferLinearDepth.Load(ss_load_coords + int3(0, 1,0)).x
		+ txGBufferLinearDepth.Load(ss_load_coords + int3(-1, 1,0)).x);

	edge = sqrt(abs(depth - average)) * _EDGE;
		
	if(edge > 0.5)
		discard;
		
	depth = saturate(2.0f * depth);
    depth = 1 - depth;
    depth *= depth;
    depth = 1 - depth;

    float samplePos = float4(1.0f * depth.xx, 0.0f, 0.0f);
    samplePos.x -= _PULSE * linear_time;
    
    float4 colour = txNoiseMap.Sample(samLinear, samplePos);
    colour *= (colour * (2.0f + edge * 30.0f) + edge * 5.0f);
    
    // Can't use sample because it's a texture of ints
    // stencil value is in the green channel
    float zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;

    if (_PULSE * linear_time > zlinear)
    {
        uint s_cc = txBackBufferStencil.Load(ss_load_coords).g;

        // n = north, s = south, e = east, w = west, c = center
        uint s_nw = txBackBufferStencil.Load(ss_load_coords + int3(1, -1, 0)).y;
        uint s_nc = txBackBufferStencil.Load(ss_load_coords + int3(0, -1, 0)).y;
        uint s_ne = txBackBufferStencil.Load(ss_load_coords + int3(-1, -1, 0)).y;
        uint s_cw = txBackBufferStencil.Load(ss_load_coords + int3(1, 0, 0)).y;

        //   s_cc
        uint s_ce = txBackBufferStencil.Load(ss_load_coords + int3(-1, 0, 0)).y;
        uint s_sw = txBackBufferStencil.Load(ss_load_coords + int3(1, 1, 0)).y;
        uint s_sc = txBackBufferStencil.Load(ss_load_coords + int3(0, 1, 0)).y;
        uint s_se = txBackBufferStencil.Load(ss_load_coords + int3(-1, 1, 0)).y;

        // We expect all the 3x3 pixels to have the same value 
        int sum_stencils = s_nw + s_nc + s_ne + s_cw + s_cc + s_ce + s_sw + s_sc + s_se;
        uint diff = sum_stencils - s_cc * 9;
        // If not we are in the border

        if (diff != 0)
            return float4(0, 1, 1, 1) * outline_alpha;
    }

    // or we are outside, all zeros.
    return float4(colour.xyz, 0.5) * outline_alpha;
}
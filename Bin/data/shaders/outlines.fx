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
    // Convert screen position to uv coordinates
    //float3 ss_test_coords = float3(iPosition.xy, 0);
    float2 uv = iPosition.xy * camera_inv_resolution;
    
    // Retrieve the linear depth on given pixel
    int3 ss_load_coords = uint3(iPosition.xy, 0);
    float depth = txGBufferLinearDepth.Load(ss_load_coords).x;

    float edge = 0;
    float _EDGE = 1.0f;
    float _PULSE = 0.2f;

    if (depth > 0.99f)
        discard;

    if (_EDGE > 0.0f) {
        float4 offset = float4(1, 1, -1, -1) * (camera_inv_resolution.xyxy);
        
        float average = 0.25f * (
              txGBufferLinearDepth.Load(float3(uv,0) + float3(offset.xy, 0))
            + txGBufferLinearDepth.Load(float3(uv,0) + float3(offset.zy, 0))
            + txGBufferLinearDepth.Load(float3(uv,0) + float3(offset.xw, 0))
            + txGBufferLinearDepth.Load(float3(uv,0) + float3(offset.zw, 0)));

        edge = sqrt(abs(depth - average)) * _EDGE;
    }

    depth = 1 - depth;
    depth *= depth;
    depth = 1 - depth;

    float samplePos = float4(1.0f * depth.xx, 0.0f, 0.0f);
    samplePos.x -= _PULSE * global_world_time;
    
    float4 colour = txNoiseMap.Sample(samLinear, samplePos);
    colour *= (colour * (2.0f + edge * 30.0f) + edge * 5.0f);

    return float4(colour.xyz, 0.5f);
    /*
    float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;

    // Can't use sample because it's a texture of ints
    // stencil value is in the green channel
    uint s_cc = txBackBufferStencil.Load(ss_load_coords).g;

    float a = mat_alpha_outline;

    // In case we want just the pixels inside
    //if( s_cc != 0 ) return float4( 1,1,0,0.5 * a); 

    // n = north, s = south, e = east, w = west, c = center
    uint s_nw = txBackBufferStencil.Load(ss_load_coords + int3(1,-1,0)).y;
    uint s_nc = txBackBufferStencil.Load(ss_load_coords + int3(0,-1,0)).y;
    uint s_ne = txBackBufferStencil.Load(ss_load_coords + int3(-1,-1,0)).y;
    uint s_cw = txBackBufferStencil.Load(ss_load_coords + int3(1,0,0)).y;
    //   s_cc
    uint s_ce = txBackBufferStencil.Load(ss_load_coords + int3(-1,0,0)).y;
    uint s_sw = txBackBufferStencil.Load(ss_load_coords + int3(1, 1,0)).y;
    uint s_sc = txBackBufferStencil.Load(ss_load_coords + int3(0, 1,0)).y;
    uint s_se = txBackBufferStencil.Load(ss_load_coords + int3(-1, 1,0)).y;

    // We expect all the 3x3 pixels to have the same value 
    int sum_stencils = s_nw + s_nc + s_ne + s_cw + s_cc + s_ce + s_sw + s_sc + s_se;
    uint diff = sum_stencils - s_cc * 9;
    // If not we are in the border
    if (diff != 0)
    return float4(1,0,0,a);

    // else, or we are inside ( stencil != 0 ) 
    //if( s_cc != 0 ) {
    // Use different stencil values for different colors!
    //return float4( 1,0,0,0.5*a); 
    //}

    // or we are outside, all zeros.
    return float4(1,0,0,0);*/
}
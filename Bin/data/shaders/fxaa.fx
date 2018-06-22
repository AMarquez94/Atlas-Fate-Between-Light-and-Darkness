#include "common.fx"


#define FXAA_PC 1
#define FXAA_HLSL_5 0
#define FXAA_HLSL_4 1
#define FXAA_QUALITY__PRESET 10
#define FXAA_GREEN_AS_LUMA 1

#include "FXAA.hlsl"

float4 PS_FXAA(
  float4 position : SV_POSITION,
  float2 texcoord : TEXCOORD0
) : SV_TARGET
{
  float2 rcp_frame = camera_inv_resolution;
  FxaaTex InputFXAATex = { samClampPoint, txAlbedo };
  return FxaaPixelShader(
    texcoord.xy,             // FxaaFloat2 pos,
    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),   // FxaaFloat4 fxaaConsolePosPos,
    InputFXAATex,             // FxaaTex tex,
    InputFXAATex,             // FxaaTex fxaaConsole360TexExpBiasNegOne,
    InputFXAATex,             // FxaaTex fxaaConsole360TexExpBiasNegTwo,
    rcp_frame,                // FxaaFloat2 fxaaQualityRcpFrame,
    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),   // FxaaFloat4 fxaaConsoleRcpFrameOpt,
    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),   // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),   // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
    0.75f,                // FxaaFloat fxaaQualitySubpix,
    0.166f,               // FxaaFloat fxaaQualityEdgeThreshold,
    0.0833f,             // FxaaFloat fxaaQualityEdgeThresholdMin,
    0.0f,                 // FxaaFloat fxaaConsoleEdgeSharpness,
    0.0f,                 // FxaaFloat fxaaConsoleEdgeThreshold,
    0.0f,                 // FxaaFloat fxaaConsoleEdgeThresholdMin,
    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f)    // FxaaFloat fxaaConsole360ConstDir,
  );
}
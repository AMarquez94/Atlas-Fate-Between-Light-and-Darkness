#include "mcv_platform.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler")

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(
    const char* szFileName
  , const char* szEntryPoint
  , const char* szShaderModel     // vs_4_0, ps_5_0, cs..
  , ID3DBlob** ppBlobOut)
{
  HRESULT hr = S_OK;

  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

  // To avoid requiring using the transpose when sending matrix to the ctes
  dwShaderFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

#if defined( DEBUG ) || defined( _DEBUG )
  // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
  // Setting this flag improves the shader debugging experience, but still allows 
  // the shaders to be optimized and to run exactly the way they will run in 
  // the release configuration of this program.
  dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

  // Convert from multibyte (1 o +) to wide char(always 2)
  WCHAR wFilename[MAX_PATH];
  mbstowcs(wFilename, szFileName, strlen(szFileName)+1);

  ID3DBlob* pErrorBlob = nullptr;
  hr = D3DCompileFromFile(wFilename,
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE,
    szEntryPoint,
    szShaderModel,
    dwShaderFlags,
    0,
    ppBlobOut,
    &pErrorBlob
  );

  if (FAILED(hr))
  {
    if (pErrorBlob != NULL) {
      fatal("Error compiling shader: %s ", (char*)pErrorBlob->GetBufferPointer());
    }
    if (pErrorBlob) pErrorBlob->Release();
    return hr;
  }
  if (pErrorBlob) pErrorBlob->Release();

  return S_OK;
}
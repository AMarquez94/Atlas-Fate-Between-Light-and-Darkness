#ifndef INC_SHADER_COMPILER_H_
#define INC_SHADER_COMPILER_H_

HRESULT CompileShaderFromFile(
  const char* szFileName
  , const char* szEntryPoint
  , const char* szShaderModel
  , ID3DBlob** ppBlobOut);

#endif




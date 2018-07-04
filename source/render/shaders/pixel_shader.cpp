#include "mcv_platform.h"
#include "pixel_shader.h"
#include "shader_compiler.h"

bool CPixelShader::create(const std::string& filename, const std::string& fn_entry_name) {

  assert( ps == nullptr );
  HRESULT hr;

  // Compile the pixel shader, convert the txt to some byte code
  ID3DBlob* pPSBlob = NULL;
  hr = CompileShaderFromFile(filename.c_str(), fn_entry_name.c_str(), "ps_5_0", &pPSBlob);
  if (FAILED(hr))
    return false;

  // Create the pixel shader in the GPU
  hr = Render.device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &ps);
  pPSBlob->Release();
  if (FAILED(hr))
    return false;

  setDXName(ps, (filename + ":" + fn_entry_name).c_str());

  return true;
}

void CPixelShader::destroy() {
  SAFE_RELEASE(ps);
}

void CPixelShader::activate() const{
  assert(ps);
  Render.ctx->PSSetShader(ps, NULL, 0);
}


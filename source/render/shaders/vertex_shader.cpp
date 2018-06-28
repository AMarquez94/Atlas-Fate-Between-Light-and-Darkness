#include "mcv_platform.h"
#include "vertex_shader.h"
#include "vertex_declarations.h"
#include "shader_compiler.h"

bool CVertexShader::create(const std::string& filename, const std::string& fn_entry_name, const std::string& vtx_decl_name ) {

  assert( vs == nullptr );

  HRESULT hr;

  // Compile the vertex shader, convert the txt to some byte code
  ID3DBlob* pVSBlob = NULL;
  hr = CompileShaderFromFile(filename.c_str(), fn_entry_name.c_str(), "vs_5_0", &pVSBlob);
  if (FAILED(hr))
    return false;

  // Create the vertex shader in the GPU
  hr = Render.device->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vs);
  if (FAILED(hr))
  {
    pVSBlob->Release();
    return false;
  }

  // Save vtx_declaration used by me
  vtx_declaration = CVertexDeclManager::get().getByName(vtx_decl_name);
  assert(vtx_declaration);

  // 
  hr = Render.device->CreateInputLayout(vtx_declaration->cpu_layout, vtx_declaration->numElements, pVSBlob->GetBufferPointer(),
	  pVSBlob->GetBufferSize(), &vertex_layout);
  if (FAILED(hr)) 
  {
    pVSBlob->Release();
    return false;
  }

  // Create the input layout associated to my shader
  pVSBlob->Release();
  if (FAILED(hr))
    return false;

  setDXName(vertex_layout, vtx_decl_name.c_str());
  setDXName(vs, (filename + ":" + fn_entry_name).c_str());

  return true;

}

void CVertexShader::destroy() {
  SAFE_RELEASE(vertex_layout);
  SAFE_RELEASE(vs);
}

void CVertexShader::activate() const {

  assert(vs && vertex_layout);

  // Specify the vertex layout to the device
  Render.ctx->IASetInputLayout(vertex_layout);

  // Activate our shader
  Render.ctx->VSSetShader(vs, NULL, 0);
}


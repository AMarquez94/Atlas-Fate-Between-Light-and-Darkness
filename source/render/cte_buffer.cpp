#include "mcv_platform.h"
#include "cte_buffer.h"

bool CCteBuffer::createData(UINT num_bytes) {

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  // Create the constant buffer
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = num_bytes;
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;
  HRESULT hr = Render.device->CreateBuffer(&bd, NULL, &cb);
  if (FAILED(hr))
    return false;

  return true;
}

void CCteBuffer::destroy() {
  SAFE_RELEASE(cb);
}

void CCteBuffer::activate() {
  Render.ctx->VSSetConstantBuffers(slot, 1, &cb);
}
#ifndef INC_RENDER_H_
#define INC_RENDER_H_

class CRender {

public:
  ID3D11Device*           device = nullptr;
  ID3D11DeviceContext*    ctx = nullptr;
  ID3D11RenderTargetView* renderTargetView = nullptr;
  IDXGISwapChain*         swapChain = nullptr;

  ID3D11Texture2D*        depthTexture = nullptr;
  ID3D11DepthStencilView* depthStencilView = nullptr;
  ID3D11ShaderResourceView* depth_shader_resource_view = nullptr;

  int width = 0;
  int height = 0;

  bool createDevice(int new_width, int new_height);
  void destroyDevice();
  void startRenderInBackbuffer();
};

extern CRender Render;

#define SAFE_RELEASE(x)  if(x) x->Release(), x = nullptr;

// To assign a name to each DX object
#define setDXName(dx_obj,new_name) \
        (dx_obj)->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(new_name), new_name);

#include "render/gpu_trace.h"
#include "render/mesh/mesh.h"
#include "render/shaders/render_technique.h"
#include "render/shaders/vertex_declarations.h"

#endif



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

  int width = 0;
  int height = 0;

  bool createDevice(int new_width, int new_height);
  void destroyDevice();
  void startRenderInBackbuffer();
};

extern CRender Render;

#define SAFE_RELEASE(x)  if(x) x->Release(), x = nullptr;

#include "render/vertex_shader.h"
#include "render/pixel_shader.h"
#include "render/mesh/mesh.h"
#include "render/render_technique.h"

#endif



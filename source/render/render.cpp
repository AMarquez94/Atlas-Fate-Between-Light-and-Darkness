#include "mcv_platform.h"
#include "windows/app.h"
#include "render.h"
#include <DXGIDebug.h>

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3d9")      // For the perf funcs
#pragma comment(lib, "dxguid")    // For the setDXName

CRender Render;
ID3D11ShaderResourceView* depth_shader_resource_view = nullptr;

bool CRender::createDevice(int new_width, int new_height) {

  width = new_width;
  height = new_height;

  D3D_FEATURE_LEVEL featureLevels[] =
  {
    D3D_FEATURE_LEVEL_11_0,
  };
  UINT numFeatureLevels = ARRAYSIZE(featureLevels);

  UINT createDeviceFlags = 0;
#ifdef _DEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 1;
  sd.BufferDesc.Width = width;
  sd.BufferDesc.Height = height;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = CApp::get().getWnd();
  
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = !CApp::get().fullscreen;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
  HRESULT hr;

  hr = D3D11CreateDeviceAndSwapChain(
    NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
    createDeviceFlags, featureLevels, numFeatureLevels,
    D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &ctx);
  if (FAILED(hr))
    return false;

  // Create a render target view
  ID3D11Texture2D* pBackBuffer = NULL;
  hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
  if (FAILED(hr))
    return false;

  hr = device->CreateRenderTargetView(pBackBuffer, NULL, &renderTargetView);
  pBackBuffer->Release();
  if (FAILED(hr))
    return false;

  dbg("Render.Device created at %dx%d\n", width, height);

  // Crear un ZBuffer de la resolucion de mi backbuffer
  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  hr = Render.device->CreateTexture2D(&desc, NULL, &depthTexture);
  if (FAILED(hr))
    return false;

  // Create the depth stencil view
  D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
  ZeroMemory(&descDSV, sizeof(descDSV));
  descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  hr = Render.device->CreateDepthStencilView(depthTexture, &descDSV, &depthStencilView);
  if (FAILED(hr))
    return false;

  // -----------------------------------------
  // Create a resource view so we can use the data in a shader
  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
  ZeroMemory(&srv_desc, sizeof(srv_desc));
  srv_desc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = desc.MipLevels;
  hr = Render.device->CreateShaderResourceView(depthTexture, &srv_desc, &depth_shader_resource_view);
  if (FAILED(hr))
      return false;
  setDXName(depth_shader_resource_view, "BackBuffer-Stencil");

  // Create a ShaderResourceView of the "depth" part of the resource 
  // using the strongly typed format DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
  // Texture2D<float> depthBuffer; // Red contains depth.

  return true;
}

// -------------------------------------------------------------------
void CRender::destroyDevice() {

	swapChain->SetFullscreenState(FALSE, NULL);
  if (ctx) ctx->ClearState();
  SAFE_RELEASE(depth_shader_resource_view);
  SAFE_RELEASE(depthTexture);
  SAFE_RELEASE(depthStencilView);
  SAFE_RELEASE(renderTargetView);
  SAFE_RELEASE(swapChain);
  SAFE_RELEASE(ctx);

#ifdef _DEBUG
  if (0) {
	  ID3D11Debug* DebugDevice = nullptr;
	  HRESULT hr = device->QueryInterface(&DebugDevice);
	  if (DebugDevice)
		  DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
  }
#endif

  SAFE_RELEASE(device);
}

// -------------------------------------------------------------------
void CRender::startRenderInBackbuffer() {
  assert(ctx);

  // Activate also the depthStencil 
  ctx->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

  // Setup the viewport
  D3D11_VIEWPORT vp;
  vp.Width = (FLOAT)width;
  vp.Height = (FLOAT)height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  ctx->RSSetViewports(1, &vp);
}


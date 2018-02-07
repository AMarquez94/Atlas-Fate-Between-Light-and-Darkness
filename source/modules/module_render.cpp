#include "mcv_platform.h"
#include "module_render.h"
#include "windows/app.h"
#include "imgui/imgui_impl_dx11.h"
#include "render/render_objects.h"
#include "camera/camera.h"

//--------------------------------------------------------------------------------------
CVertexShader vs;
CPixelShader ps;
CVertexShader vs_obj;
CPixelShader ps_obj;

//--------------------------------------------------------------------------------------
CModuleRender::CModuleRender(const std::string& name)
	: IModule(name)
{}

bool CModuleRender::start()
{
  if (!Render.createDevice(_xres, _yres))
    return false;

  if (!CVertexDeclManager::get().create())
    return false;

  if (!createRenderObjects())
    return false;

  // --------------------------------------------
  // ImGui
  auto& app = CApp::get();
  if (!ImGui_ImplDX11_Init(app.getWnd(), Render.device, Render.ctx))
    return false;

  // --------------------------------------------
  // my custom code
  if (!vs.create("data/shaders/shaders.fx", "VS", "PosClr"))
    return false;

  if (!ps.create("data/shaders/shaders.fx", "PS"))
    return false; 

  if (!vs_obj.create("data/shaders/shaders_objs.fx", "VS", "PosNUv"))
    return false;

  if (!ps_obj.create("data/shaders/shaders_objs.fx", "PS"))
    return false;

  setBackgroundColor(0.0f, 0.125f, 0.3f, 1.f);

  return true;
}

// Forward the OS msg to the IMGUI
LRESULT CModuleRender::OnOSMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

bool CModuleRender::stop()
{
  ps.destroy();
  vs.destroy();

  ImGui_ImplDX11_Shutdown();

  destroyRenderObjects();

  Render.destroyDevice();
  return true;
}

void CModuleRender::update(float delta)
{
	(void)delta;
  // Notify ImGUI that we are starting a new frame
  ImGui_ImplDX11_NewFrame();
}

void CModuleRender::render()
{
  // Edit the Background color
  ImGui::ColorEdit4("Background Color", _backgroundColor);

  Render.startRenderInBackbuffer();

  // Clear the back buffer 
  float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
  Render.ctx->ClearRenderTargetView(Render.renderTargetView, _backgroundColor);
  Render.ctx->ClearDepthStencilView(Render.depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

  vs.activate();
  ps.activate();
}

void CModuleRender::configure(int xres, int yres)
{
  _xres = xres;
  _yres = yres;
}

void CModuleRender::setBackgroundColor(float r, float g, float b, float a)
{
  _backgroundColor[0] = r;
  _backgroundColor[1] = g;
  _backgroundColor[2] = b;
  _backgroundColor[3] = a;
}

void CModuleRender::generateFrame() {

  CEngine::get().getModules().render();

  ImGui::Render();

  // Present the information rendered to the back buffer to the front buffer (the screen)
  Render.swapChain->Present(0, 0);
}


#include "mcv_platform.h"
#include "module_render.h"
#include "windows/app.h"
#include "imgui/imgui_impl_dx11.h"
#include "render/render_objects.h"
#include "render/render_utils.h"
#include "render/render_manager.h"
#include "components/comp_light.h"
#include "render/texture/material.h"
#include "render/texture/texture.h"
#include "resources/json_resource.h"
#include "components/skeleton/game_core_skeleton.h"
#include "physics/physics_mesh.h"
#include "camera/camera.h"
#include "geometry/curve.h"
#include "fsm/fsm.h"

//--------------------------------------------------------------------------------------

CModuleRender::CModuleRender(const std::string& name)
	: IModule(name)
{}

//--------------------------------------------------------------------------------------
// All techs are loaded from this json file
bool parseTechniques() {
	json j = loadJson("data/techniques.json");
	for (auto it = j.begin(); it != j.end(); ++it) {

		std::string tech_name = it.key() + ".tech";
		json& tech_j = it.value();

		CRenderTechnique* tech = new CRenderTechnique();
		if (!tech->create(tech_name, tech_j))
			return false;
		Resources.registerResource(tech);
	}

	return true;
}

bool CModuleRender::start()
{
	if (!Render.createDevice(_xres, _yres))
		return false;

	if (!CVertexDeclManager::get().create())
		return false;

	// Register the resource types
	Resources.registerResourceClass(getResourceClassOf<CJsonResource>());
	Resources.registerResourceClass(getResourceClassOf<CTexture>());
	Resources.registerResourceClass(getResourceClassOf<CRenderMesh>());
	Resources.registerResourceClass(getResourceClassOf<CRenderTechnique>());
	Resources.registerResourceClass(getResourceClassOf<CMaterial>());
	Resources.registerResourceClass(getResourceClassOf<CGameCoreSkeleton>());
	Resources.registerResourceClass(getResourceClassOf<CPhysicsMesh>());
	Resources.registerResourceClass(getResourceClassOf<CCurve>());


	if (!createRenderObjects())
		return false;

	if (!createRenderUtils())
		return false;

	// --------------------------------------------
	// ImGui
	auto& app = CApp::get();
	if (!ImGui_ImplDX11_Init(app.getWnd(), Render.device, Render.ctx))
		return false;

	if (!parseTechniques())
		return false;

	setBackgroundColor(0.0f, 0.125f, 0.3f, 1.f);

	camera.lookAt(VEC3(12.0f, 8.0f, 8.0f), VEC3::Zero, VEC3::UnitY);
	camera.setPerspective(60.0f * 180.f / (float)M_PI, 0.1f, 1000.f);

	return true;
}

// Forward the OS msg to the IMGUI
LRESULT CModuleRender::OnOSMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

bool CModuleRender::stop()
{
	ImGui_ImplDX11_Shutdown();

	destroyRenderUtils();
	destroyRenderObjects();

	Resources.destroyAll();

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
	static int nframes = 5;
	ImGui::DragInt("NumFrames To Capture", &nframes, 0.1f, 1, 20);
	if (ImGui::SmallButton("Start CPU Trace Capturing")) {
		PROFILE_SET_NFRAMES(nframes);
	}

	// Edit the Background color
	ImGui::ColorEdit4("Background Color", _backgroundColor);
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

void CModuleRender::activateMainCamera() {

	CCamera* cam = &camera;

	CHandle h_e_camera = getEntityByName("TPCamera");
	if (h_e_camera.isValid()) {
		CEntity* e_camera = h_e_camera;
		TCompCamera* c_camera = e_camera->get< TCompCamera >();
		cam = c_camera;
		assert(cam);
		CRenderManager::get().setEntityCamera(h_e_camera);
	}

	activateCamera(*cam, Render.width, Render.height);
}

void CModuleRender::generateFrame() {

	{
		PROFILE_FUNCTION("CModuleRender::shadowsMapsGeneration");
		CTraceScoped gpu_scope("shadowsMapsGeneration");
		// Generate the shadow map for each active light
		getObjectManager<TCompLight>()->forEach([](TCompLight* c) {
			c->generateShadowMap();
		});
	}

	{
		CTraceScoped gpu_scope("Frame");
		PROFILE_FUNCTION("CModuleRender::generateFrame");
		Render.startRenderInBackbuffer();

		// Clear the back buffer 
		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
		Render.ctx->ClearRenderTargetView(Render.renderTargetView, _backgroundColor);
		Render.ctx->ClearDepthStencilView(Render.depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		activateMainCamera();

		getObjectManager<TCompLight>()->forEach([](TCompLight* c) {
			c->activate();
		});

		CRenderManager::get().renderCategory("default");

		// Debug render
		{
			PROFILE_FUNCTION("Modules");
			CTraceScoped gpu_scope("Modules");
			CEngine::get().getModules().render();
		}
	}

	{
		PROFILE_FUNCTION("ImGui::Render");
		CTraceScoped gpu_scope("ImGui");
		ImGui::Render();
	}

	// Present the information rendered to the back buffer to the front buffer (the screen)
	{
		PROFILE_FUNCTION("Render.swapChain");
		Render.swapChain->Present(0, 0);
	}

	// Present the information rendered to the back buffer to the front buffer (the screen)
	{
		PROFILE_FUNCTION("Render.swapChain");
		Render.swapChain->Present(0, 0);
	}
}


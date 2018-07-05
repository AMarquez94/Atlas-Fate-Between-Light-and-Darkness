#include "mcv_platform.h"
#include "module_render.h"
#include "windows/app.h"
#include "imgui/imgui_impl_dx11.h"
#include "render/render_objects.h"
#include "render/render_utils.h"
#include "render/render_manager.h"
#include "components/lighting/comp_light_dir.h"
#include "components/lighting/comp_light_spot.h"
#include "render/texture/material.h"
#include "render/texture/texture.h"
#include "resources/json_resource.h"
#include "components/skeleton/game_core_skeleton.h"
#include "components/postfx/comp_render_outlines.h"
#include "physics/physics_mesh.h"
#include "camera/camera.h"
#include "geometry/curve.h"
#include "fsm/fsm.h"

#include "geometry/geometry.h"
#include "render/texture/render_to_texture.h"
#include "components/postfx/comp_render_blur.h"
#include "components/postfx/comp_render_blur_radial.h"
#include "components/postfx/comp_render_bloom.h"
#include "components/postfx/comp_color_grading.h"
#include "components/postfx/comp_fog.h"
#include "components/postfx/comp_antialiasing.h"
#include "components/postfx/comp_chrom_aberration.h"
#include "components\comp_render.h"
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
		if (!tech->create(tech_name, tech_j)) {
			fatal("Failed to create tech '%s'\n", tech_name.c_str());
			return false;
		}
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

	rt_main = new CRenderToTexture;
	if (!rt_main->createRT("rt_main.dds", Render.width, Render.height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, true))
		return false;

	if (!deferred.create(Render.width, Render.height))
		return false;

	setBackgroundColor(0.0f, 0.125f, 0.3f, 1.f);

	// -------------------------------------------
	if (!cb_camera.create(CB_CAMERA))
		return false;
	// -------------------------------------------
	if (!cb_object.create(CB_OBJECT))
		return false;

	if (!cb_light.create(CB_LIGHT))
		return false;

	if (!cb_globals.create(CB_GLOBALS))
		return false;

	// Post processing effects
	if (!cb_blur.create(CB_BLUR))
		return false;

	if (!cb_gui.create(CB_GUI))
		return false;

    if (!cb_particles.create(CB_PARTICLE))
        return false;

    if (!cb_outline.create(CB_OUTLINE))
        return false;

    if (!cb_player.create(CB_PLAYER))
        return false;

    if (!cb_postfx.create(CB_POSTFX))
        return false;

	cb_globals.global_exposure_adjustment = 2.010f;
	cb_globals.global_ambient_adjustment = 0.150f;
	cb_globals.global_world_time = 0.f;
	cb_globals.global_hdr_enabled = 1.f;
	cb_globals.global_gamma_correction_enabled = 1.f;
	cb_globals.global_tone_mapping_mode = 1.f;
    cb_globals.global_fog_density = 0.017f;
    cb_globals.global_fog_color = VEC3(0.18,0.5,0.7);
    cb_globals.global_self_intensity = 10.f;

	cb_light.activate();
	cb_object.activate();
	cb_camera.activate();
	cb_globals.activate();
	cb_blur.activate();
	cb_gui.activate();
    cb_particles.activate();
    cb_outline.activate();
    cb_player.activate();
    cb_postfx.activate();

	camera.lookAt(VEC3(12.0f, 8.0f, 8.0f), VEC3::Zero, VEC3::UnitY);
	camera.setPerspective(60.0f * 180.f / (float)M_PI, 0.1f, 1000.f);

	return true;
}

// Forward the OS msg to the IMGUI
LRESULT CModuleRender::OnOSMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

CHandle CModuleRender::getMainCamera() {

    return h_e_camera;
}

bool CModuleRender::stop()
{
	ImGui_ImplDX11_Shutdown();

	destroyRenderUtils();
	destroyRenderObjects();
	Resources.destroyAll();
	Render.destroyDevice();

	cb_camera.destroy();
	cb_object.destroy();
	cb_light.destroy();
	cb_globals.destroy();
	cb_blur.destroy();
    cb_gui.destroy();
    cb_particles.destroy();
    cb_outline.destroy();
    cb_player.destroy();
    cb_postfx.destroy();

	return true;
}

void CModuleRender::update(float delta)
{
	(void)delta;
	// Notify ImGUI that we are starting a new frame
	ImGui_ImplDX11_NewFrame();

	cb_globals.global_world_time += delta;
}

void CModuleRender::render()
{
	if (ImGui::TreeNode("Profiler")) {

		static int nframes = 5;
		ImGui::DragInt("NumFrames To Capture", &nframes, 0.1f, 1, 20);
		if (ImGui::SmallButton("Start CPU Trace Capturing")) {
			PROFILE_SET_NFRAMES(nframes);
		}

		// Edit the Background color
		ImGui::DragFloat("Time Factor", &EngineEntities.time_scale_factor, 0.01f, 0.f, 1.0f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Lighting")) {

		ImGui::DragFloat("Exposure Adjustment", &cb_globals.global_exposure_adjustment, 0.01f, 0.1f, 32.f);
		ImGui::DragFloat("Ambient Adjustment", &cb_globals.global_ambient_adjustment, 0.01f, 0.0f, 1.f);
		ImGui::DragFloat("HDR", &cb_globals.global_hdr_enabled, 0.01f, 0.0f, 1.f);
		ImGui::DragFloat("Gamma Correction", &cb_globals.global_gamma_correction_enabled, 0.01f, 0.0f, 1.f);
		ImGui::DragFloat("Reinhard vs Uncharted2", &cb_globals.global_tone_mapping_mode, 0.01f, 0.0f, 1.f);
        ImGui::DragFloat("Fog density", &cb_globals.global_fog_density, 0.0001f, 0.0f, 1.f);
        ImGui::ColorEdit4("Fog Color", &cb_globals.global_fog_color.x, 0.0001f);

		// Must be in the same order as the RO_* ctes
		static const char* render_output_str =
			"Complete\0"
			"Albedo\0"
			"Normal\0"
			"Roughness\0"
			"Metallic\0"
			"World Pos\0"
			"Depth Linear\0"
			"AO\0"
			"\0";

		ImGui::Combo("Output", &cb_globals.global_render_output, render_output_str);
		ImGui::Checkbox("Wireframe", &_showWireframe);
		ImGui::TreePop();
	}

	ImGui::Separator();
}

void CModuleRender::configure(int xres, int yres)
{
	_xres = xres;
	_yres = yres;
}

void CModuleRender::setBackgroundColor(float r, float g, float b, float a)
{
	_backgroundColor.x = r;
	_backgroundColor.y = g;
	_backgroundColor.z = b;
	_backgroundColor.w = a;
}

void CModuleRender::activateMainCamera() {

	CCamera* cam = &camera;

	h_e_camera = getEntityByName("main_camera");
	if (h_e_camera.isValid()) {
		CEntity* e_camera = h_e_camera;
		TCompCamera* c_camera = e_camera->get< TCompCamera >();
		cam = c_camera;
		assert(cam);
		CRenderManager::get().setEntityCamera(h_e_camera);
	}

	activateCamera(*cam, Render.width, Render.height);
}

void CModuleRender::renderWireframeLayer(bool hideBackground) {
	CTraceScoped gpu_scope("renderWireframeLayer");
	PROFILE_FUNCTION("renderWireframeLayer");
	if (hideBackground) {
		/*Render.ctx->ClearRenderTargetView(Render.renderTargetView, &_backgroundColor.x);
		Render.ctx->ClearDepthStencilView(Render.depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);*/
	}
	auto solid = Resources.get("data/materials/solid.material")->as<CMaterial>();
	solid->activate();
	getObjectManager<TCompRender>()->forEach([](TCompRender* c) {
		c->renderDebug();
	});

}

void CModuleRender::renderCollidersLayer(bool onlyDynamics) {
	CTraceScoped gpu_scope("renderColliderLayer");
	PROFILE_FUNCTION("renderColliderLayer");
	getObjectManager<TCompCollider>()->forEach([onlyDynamics](TCompCollider* c) {
		c->renderDebug(onlyDynamics);
	});
}

void CModuleRender::generateFrame() {

    {
        // SHADOW GENERATION
        activateMainCamera();
        PROFILE_FUNCTION("CModuleRender::shadowsMapsGeneration");
        CTraceScoped gpu_scope("shadowsMapsGeneration");
        if (_generateShadows) {
            // Generate the shadow map for each active light
            getObjectManager<TCompLightDir>()->forEach([](TCompLightDir* c) {
                c->generateShadowMap();
            });

            // Generate the shadow map for each active light
            getObjectManager<TCompLightSpot>()->forEach([](TCompLightSpot* c) {
                c->cullFrame();
                c->generateShadowMap();
            });
        }
    }

    {
        // MAIN FRAME RENDER
        CTraceScoped gpu_scope("Frame");
        PROFILE_FUNCTION("CModuleRender::generateFrame");

        activateMainCamera();
        cb_globals.updateGPU();
        deferred.render(rt_main, h_e_camera);

        CRenderManager::get().renderCategory("particles");
        CRenderManager::get().renderCategory("distorsions");
    }

    {
        // POST PROCESSING STACK
        CTexture * curr_rt = rt_main;
        CHandle camera_render = Engine.getCameras().getCurrentCamera();

        if (camera_render.isValid() && _generatePostFX) {
            CEntity * e_cam = camera_render;

            // The bloom blurs the given input
            TCompRenderBloom* c_render_bloom = e_cam->get< TCompRenderBloom >();
            if (c_render_bloom) {
                c_render_bloom->generateHighlights(deferred.rt_acc_light);
                c_render_bloom->addBloom();
            }

            TCompRenderBlur * c_render_blur = e_cam->get< TCompRenderBlur >();
            if (c_render_blur)
                curr_rt = c_render_blur->apply(curr_rt);

            // Check if we have a render_fx component
            TCompRenderBlurRadial * c_render_blur_radial = e_cam->get< TCompRenderBlurRadial >();
            if (c_render_blur_radial)
                curr_rt = c_render_blur_radial->apply(curr_rt);

			// Check if we have a color grading component
			TCompColorGrading* c_color_grading = e_cam->get< TCompColorGrading >();
			if (c_color_grading)
				curr_rt = c_color_grading->apply(curr_rt);

            // Check if we have a color grading component
            TCompFog * c_render_fog = e_cam->get< TCompFog >();
            if (c_render_fog)
                curr_rt = c_render_fog->apply(curr_rt);

            TCompChromaticAberration* c_chroma_aberration = e_cam->get< TCompChromaticAberration >();
            if (c_chroma_aberration)
                curr_rt = c_chroma_aberration->apply(curr_rt);

            TCompRenderOutlines* c_render_outlines = e_cam->get< TCompRenderOutlines >();
            if (c_render_outlines)
                c_render_outlines->apply();

            TCompAntiAliasing* c_antialiasing = e_cam->get< TCompAntiAliasing >();
            if (c_antialiasing)
                curr_rt = c_antialiasing->apply(curr_rt);
        }

        Render.startRenderInBackbuffer();
        renderFullScreenQuad("dump_texture.tech", curr_rt);
    }

    {
        // DEBUG DRAWING
        auto* tech = Resources.get("solid.tech")->as<CRenderTechnique>();
        assert(tech);
        tech->activate();

	if (_debugMode)
		debugDraw();

	if (_showWireframe)
		renderWireframeLayer(_hideBackground);

	if (_showAllColliders) {

		renderCollidersLayer(false);
	}
	else if (_showDynamicColliders) {

		renderCollidersLayer(true);
	}

	// Finally render it
	{
		PROFILE_FUNCTION("ImGui::Render");
		CTraceScoped gpu_scope("ImGui");
		ImGui::Render();
	}

        {
            // RENDER UI
            PROFILE_FUNCTION("GUI");
            CTraceScoped gpu_scope("GUI");

            activateRSConfig(RSCFG_CULL_NONE);
            activateZConfig(ZCFG_DISABLE_ALL);
            activateBlendConfig(BLEND_CFG_COMBINATIVE);

            activateCamera(CEngine::get().getGUI().getCamera(), Render.width, Render.height);
            CEngine::get().getModules().renderGUI();

            activateRSConfig(RSCFG_DEFAULT);
            activateZConfig(ZCFG_DEFAULT);
            activateBlendConfig(BLEND_CFG_DEFAULT);
        }

        {
            // BACKBUFFER SWAPPING
            PROFILE_FUNCTION("Render.swapChain");
            Render.swapChain->Present(0, 0);
        }
    }
}

void CModuleRender::debugDraw() {

    {
        // Main Inspector window
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.149f, 0.1607f, 0.188f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 255.0f, 255.0f, 255.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0, 0.0f, 0.0f, 0.75f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0, 0.0f, 0.0f, 0.75f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.219f, 0.349f, 0.501f, 0.75f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);

        // Render each render GUI explicit in order.
        ImGui::Begin("Inspector", NULL);
        {
            PROFILE_FUNCTION("Modules");
            CTraceScoped gpu_scope("Modules");
            CEngine::get().getModules().render();
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(5);
    }

    {
        //Particle editor

    }
}
#include "mcv_platform.h"
#include "deferred_renderer.h"
#include "render_manager.h"
#include "render_utils.h"
#include "render_objects.h"
#include "resources/resources_manager.h"
#include "components/lighting/comp_light_dir.h"
#include "components/lighting/comp_light_point.h"
#include "components/lighting/comp_light_spot.h"
#include "components/lighting/comp_projector.h"
#include "components/postfx/comp_render_ao.h"
#include "render/texture/render_to_cube.h"
#include "components/comp_transform.h"
#include "ctes.h"

void CDeferredRenderer::renderGBuffer() {

	CTraceScoped gpu_scope("Deferred.GBuffer");

	// Disable the gbuffer textures as we are going to update them
	// Can't render to those textures and have them active in some slot...
	CTexture::setNullTexture(TS_DEFERRED_ALBEDOS);
	CTexture::setNullTexture(TS_DEFERRED_NORMALS);
	CTexture::setNullTexture(TS_DEFERRED_LINEAR_DEPTH);
	CTexture::setNullTexture(TS_DEFERRED_SELF_ILLUMINATION);
    CTexture::setNullTexture(TS_DEFERRED_OUTLINE);

	// Activate el multi-render-target MRT
	const int nrender_targets = 5;
	ID3D11RenderTargetView* rts[nrender_targets] = {
	  rt_albedos->getRenderTargetView(),
	  rt_normals->getRenderTargetView(),
	  rt_depth->getRenderTargetView(),
	  rt_self_illum->getRenderTargetView(),
      rt_outline->getRenderTargetView(),
	};

	// We use our 3 rt's and the Zbuffer of the backbuffer
	Render.ctx->OMSetRenderTargets(nrender_targets, rts, rt_acc_light->getDepthStencilView());
	rt_albedos->activateViewport();   // Any rt will do...

	// Clear output buffers, some can be removed if we intend to fill all the screen
	// with new data.
	rt_albedos->clear(VEC4(0, 0, 0, 1));
	rt_normals->clear(VEC4(0, 0, 1, 1));
	rt_depth->clear(VEC4(1, 1, 1, 1));
	rt_self_illum->clear(VEC4(0, 0, 0, 1));
    rt_outline->clear(VEC4(1, 1, 1, 1));

	// Clear ZBuffer with the value 1.0 (far)
    Render.ctx->ClearDepthStencilView(rt_acc_light->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the solid objects that output to the G-Buffer
	CRenderManager::get().renderCategory("gbuffer");

	// Disable rendering to all render targets.
	ID3D11RenderTargetView* rt_nulls[nrender_targets];
	for (int i = 0; i < nrender_targets; ++i) rt_nulls[i] = nullptr;
	Render.ctx->OMSetRenderTargets(nrender_targets, rt_nulls, nullptr);

	// Activate the gbuffer textures to other shaders
	rt_albedos->activate(TS_DEFERRED_ALBEDOS);
	rt_normals->activate(TS_DEFERRED_NORMALS);
	rt_depth->activate(TS_DEFERRED_LINEAR_DEPTH);
    rt_self_illum->activate(TS_DEFERRED_SELF_ILLUMINATION);
    rt_outline->activate(TS_DEFERRED_OUTLINE);
}

// -----------------------------------------------------------------
bool CDeferredRenderer::create(int xres, int yres, const char* prefix) {

    char name[64];

    rt_albedos = new CRenderToTexture;
    sprintf(name, "%s_albedos", prefix);
    if (!rt_albedos->createRT(name, xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM))
        return false;

    rt_normals = new CRenderToTexture;
    sprintf(name, "%s_normals", prefix);
    if (!rt_normals->createRT(name, xres, yres, DXGI_FORMAT_R16G16B16A16_UNORM))                                                                                 
        return false;

    rt_depth = new CRenderToTexture;
    sprintf(name, "%s_depths", prefix);
    if (!rt_depth->createRT(name, xres, yres, DXGI_FORMAT_R32_FLOAT))
        return false;

    rt_acc_light = new CRenderToTexture;
    sprintf(name, "%s_acc_light", prefix);
    bool use_back_buffer = (xres == Render.width && yres == Render.height);
    DXGI_FORMAT depth_fmt = use_back_buffer ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R32_TYPELESS;
    if (!rt_acc_light->createRT(name, xres, yres, DXGI_FORMAT_R16G16B16A16_FLOAT, depth_fmt, use_back_buffer))
        return false;

    rt_self_illum = new CRenderToTexture;
    sprintf(name, "%s_self_illum", prefix);
    if (!rt_self_illum->createRT(name, xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM))
        return false;

    rt_prev_acc_light = new CRenderToTexture;
    sprintf(name, "%s_prev_acc_light", prefix);
    if (!rt_prev_acc_light->createRT(name, xres, yres, DXGI_FORMAT_R16G16B16A16_FLOAT, depth_fmt, use_back_buffer))
        return false;

    rt_outline = new CRenderToTexture;
    sprintf(name, "%s_outline", prefix);
    if (!rt_outline->createRT(name, xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM))
        return false;

	return true;
}

// -----------------------------------------------------------------
void CDeferredRenderer::renderAmbientPass() {

	CTraceScoped gpu_scope("renderAmbientPass");
	renderFullScreenQuad("pbr_ambient.tech", nullptr);
}

void CDeferredRenderer::renderSkyBox() const {

	CTraceScoped gpu_scope("renderSkyBox");
	renderFullScreenQuad("pbr_skybox.tech", nullptr);
}

// -------------------------------------------------------------------------
void CDeferredRenderer::renderAccLight() {

	CTraceScoped gpu_scope("Deferred.AccLight");
	rt_acc_light->activateRT();
	rt_acc_light->clear(VEC4(0, 0, 0, 0));
	renderAmbientPass();
	renderPointLights();
	renderSpotLights();
    renderProjectors();
	renderDirectionalLights();
	renderSkyBox();

    CRenderManager::get().renderCategory("hologram_screen");
    CRenderManager::get().renderCategory("distorsions");

    CRenderManager::get().renderCategory("hologram");
    CRenderManager::get().renderCategory("hologram_sw");
    //CRenderManager::get().renderCategory("hologram_screen");
    CRenderManager::get().renderCategory("volume_shafts");
    CRenderManager::get().renderCategory("volume_ray_shafts");
    CRenderManager::get().renderCategory("cp_particles");
    Engine.get().getParticles().renderAdditive();
}

// -------------------------------------------------------------------------
void CDeferredRenderer::renderPointLights() {

	CTraceScoped gpu_scope("renderPointLights");

	// Activate tech for the light dir 
	auto* tech = Resources.get("pbr_point_lights.tech")->as<CRenderTechnique>();
	tech->activate();

	// All light directional use the same mesh
	auto* mesh = Resources.get("data/meshes/UnitSphere.mesh")->as<CRenderMesh>();
	mesh->activate();

	// Para todas las luces... pintala
	getObjectManager<TCompLightPoint>()->forEach([mesh](TCompLightPoint* c) {

        c->cullFrame();
        if (c->isEnabled && !c->isCulled()) {
            c->activate();
            setWorldTransform(c->getWorld());
            mesh->render();
        }
	});
}


// -------------------------------------------------------------------------
void CDeferredRenderer::renderDirectionalLights() {
    /*
    auto* tech = Resources.get("pbr_ray_shafts.tech")->as<CRenderTechnique>();
    tech->activate();

    // All light directional use the same mesh
    auto* mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
    mesh->activate();

    // Para todas las luces... pintala
    getObjectManager<TCompLightDir>()->forEach([mesh](TCompLightDir* c) {

        if (c->isEnabled) {
            c->activate();
            setWorldTransform(c->getViewProjection().Invert());
            mesh->render();
        }
    });*/

	CTraceScoped gpu_scope("renderDirectionalLights");

	// Activate tech for the light dir 
    auto* tech = Resources.get("pbr_dir_lights.tech")->as<CRenderTechnique>();
	tech->activate();

	// All light directional use the same mesh
    auto* mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
	mesh->activate();

	// Para todas las luces... pintala
	getObjectManager<TCompLightDir>()->forEach([mesh](TCompLightDir* c) {

		// subir las contantes de la posicion/dir
		// activar el shadow map...
		c->activate();

		setWorldTransform(c->getViewProjection().Invert());

		// mandar a pintar una geometria que refleje los pixeles que potencialmente
		// puede iluminar esta luz.... El Frustum solido
		mesh->render();
	});
}

// -------------------------------------------------------------------------
void CDeferredRenderer::renderSpotLights() {

	CTraceScoped gpu_scope("renderSpotLights");

	// Activate tech for the light dir 
	auto* tech = Resources.get("pbr_spot_lights.tech")->as<CRenderTechnique>();
	tech->activate();

	// All light directional use the same mesh
	auto* mesh = Resources.get("data/meshes/UnitCone.mesh")->as<CRenderMesh>();
	mesh->activate();

	// Para todas las luces... pintala
	getObjectManager<TCompLightSpot>()->forEach([mesh](TCompLightSpot* c) {

        if (c->isEnabled && !c->isCulled()) {
            c->activate();
            setWorldTransform(c->getWorld());
            mesh->render();
        }
	});
}

// Optimize this in the next milestone, use CS
// -------------------------------------------------------------------------
void CDeferredRenderer::renderVolumes() {

    CTraceScoped gpu_scope("renderVolumes");
    /*
    // Activate tech for the light dir 
    auto* tech = Resources.get("pbr_ray_shafts.tech")->as<CRenderTechnique>();
    tech->activate();

    // All light directional use the same mesh
    auto* mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
    mesh->activate();

    // Para todas las luces... pintala
    getObjectManager<TCompLightSpot>()->forEach([mesh](TCompLightSpot* c) {

        if (c->isEnabled && !c->isCulled()) {
            c->activate();
		    setWorldTransform(c->getViewProjection().Invert());
            mesh->render();
        }
    });
    */
    
    auto rmesh = Resources.get("data/meshes/quad_volume.instanced_mesh")->as<CRenderMesh>();
    TCompLightSpot::volume_instance = (CRenderMeshInstanced*)rmesh;
    TCompLightSpot::volume_instance->vtx_decl = CVertexDeclManager::get().getByName("InstanceLight");

    getObjectManager<TCompLightSpot>()->forEach([](TCompLightSpot* c) {

        c->activate();
        c->generateVolume();
    });
}

// --------------------------------------
void CDeferredRenderer::renderAO(CHandle h_camera) const {

    CEntity* e_camera = h_camera;
    if (e_camera) {
        TCompRenderAO* comp_ao = e_camera->get<TCompRenderAO>();

        if (comp_ao) {
            // As we are going to update the RenderTarget AO
            // it can NOT be active as a texture while updating it.
            CTexture::setNullTexture(TS_DEFERRED_AO);
            auto ao = comp_ao->compute(rt_depth);
            // Activate the updated AO texture so everybody else can use it
            // Like the AccLight (Ambient pass or the debugger)
            ao->activate(TS_DEFERRED_AO);
            return;
        }
    }

    // As there is no comp AO, use a white texture as substitute
    const CTexture* white_texture = Resources.get("data/textures/white.dds")->as<CTexture>();
    white_texture->activate(TS_DEFERRED_AO);
}

// --------------------------------------
void CDeferredRenderer::renderProjectors() {
  // Activate tech for the light dir 
  auto* tech = Resources.get("pbr_projection.tech")->as<CRenderTechnique>();
  tech->activate();

  // All light directional use the same mesh
  auto* mesh = Resources.get("data/meshes/UnitFrustum.mesh")->as<CRenderMesh>();
  mesh->activate();

  // Para todas las luces... pintala
  getObjectManager<TCompProjector>()->forEach([mesh](TCompProjector* c) {

    c->activate();
    setWorldTransform(c->getViewProjection().Invert());
    mesh->render();
  });
}

// --------------------------------------------------------------
void CDeferredRenderer::renderGBufferDecals() {

    CTraceScoped gpu_scope("Deferred.GBuffer.Decals");

    // Disable the gbuffer textures as we are going to update them
    // Can't render to those textures and have them active in some slot...
    CTexture::setNullTexture(TS_DEFERRED_ALBEDOS);
    CTexture::setNullTexture(TS_DEFERRED_NORMALS);

    // Activate el multi-render-target MRT
    const int nrender_targets = 2;
    ID3D11RenderTargetView* rts[nrender_targets] = {
        rt_albedos->getRenderTargetView(),
        rt_normals->getRenderTargetView()
        // No Z as we need to read to reconstruct the position
    };

    // We use our 3 rt's and the Zbuffer of the backbuffer
    Render.ctx->OMSetRenderTargets(nrender_targets, rts, rt_acc_light->getDepthStencilView());
    rt_albedos->activateViewport(); // Any rt will do...

    // Render blending layer on top of gbuffer before adding lights
    CRenderManager::get().renderCategory("gbuffer_decals");

    // Disable rendering to all render targets.
    ID3D11RenderTargetView* rt_nulls[nrender_targets];
    for (int i = 0; i < nrender_targets; ++i) rt_nulls[i] = nullptr;
    Render.ctx->OMSetRenderTargets(nrender_targets, rt_nulls, nullptr);

    // Activate the gbuffer textures to other shaders
    rt_albedos->activate(TS_DEFERRED_ALBEDOS);
    rt_normals->activate(TS_DEFERRED_NORMALS);
}

// --------------------------------------------------------------
void CDeferredRenderer::renderGBufferParticles(CRenderToTexture* rt_destination) {

    CTraceScoped gpu_scope("Deferred.GBuffer.Particles");

    // Disable the gbuffer textures as we are going to update them
    // Can't render to those textures and have them active in some slot...
    CTexture::setNullTexture(TS_DEFERRED_LINEAR_DEPTH);

    // Activate el multi-render-target MRT
    const int nrender_targets = 3;
    ID3D11RenderTargetView* rts[nrender_targets] = {
        rt_destination->getRenderTargetView(),
        rt_depth->getRenderTargetView()
        // No Z as we need to read to reconstruct the position
    };

    // We use our 3 rt's and the Zbuffer of the backbuffer
    Render.ctx->OMSetRenderTargets(nrender_targets, rts, rt_acc_light->getDepthStencilView());
    rt_albedos->activateViewport(); // Any rt will do...

    Engine.get().getParticles().renderAdditive();
    //CRenderManager::get().renderCategory("cp_particles");

    // Disable rendering to all render targets.
    ID3D11RenderTargetView* rt_nulls[nrender_targets];
    for (int i = 0; i < nrender_targets; ++i) rt_nulls[i] = nullptr;
    Render.ctx->OMSetRenderTargets(nrender_targets, rt_nulls, nullptr);

    // Activate the gbuffer textures to other shaders
    rt_depth->activate(TS_DEFERRED_LINEAR_DEPTH);
}

// --------------------------------------
void CDeferredRenderer::render(CRenderToTexture* rt_destination, CHandle h_camera) {

	assert(rt_destination);

    renderGBuffer();
    renderGBufferDecals();
	renderAO(h_camera);

	// Do the same with the acc light
	CTexture::setNullTexture(TS_DEFERRED_ACC_LIGHTS);
	renderAccLight();
    renderVolumes();

	// Now dump contents to the destination buffer.
	rt_destination->activateRT();
	rt_acc_light->activate(TS_DEFERRED_ACC_LIGHTS);
    //rt_prev_acc_light->activate(TS_LUMINANCE);

	// Combine the results
	renderFullScreenQuad("gbuffer_resolve.tech", nullptr);
    //rt_prev_acc_light = rt_acc_light;
}

// --------------------------------------
void CDeferredRenderer::renderToCubeFace(CRenderToCube* rt_destination, int face_idx) {
    assert(rt_destination);

    CCamera camera;
    rt_destination->getCamera(face_idx, &camera);
    activateCamera(camera, rt_destination->getWidth(), rt_destination->getHeight());

    renderGBuffer();
    renderGBufferDecals();
    renderAO(CHandle());

    // Do the same with the acc light
    CTexture::setNullTexture(TS_DEFERRED_ACC_LIGHTS);
    renderAccLight();

    // Now dump contents to the destination buffer.
    rt_destination->activateFace(face_idx, &camera);
    rt_acc_light->activate(TS_DEFERRED_ACC_LIGHTS);

    // Combine the results
    renderFullScreenQuad("gbuffer_resolve_face.tech", nullptr);
}

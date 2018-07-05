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
#include "components/comp_transform.h"
#include "ctes.h"
#include "components/comp_aabb.h" 
#include "components/comp_culling.h" 

void CDeferredRenderer::renderGBuffer() {

	CTraceScoped gpu_scope("Deferred.GBuffer");

	// Disable the gbuffer textures as we are going to update them
	// Can't render to those textures and have them active in some slot...
	CTexture::setNullTexture(TS_DEFERRED_ALBEDOS);
	CTexture::setNullTexture(TS_DEFERRED_NORMALS);
	CTexture::setNullTexture(TS_DEFERRED_LINEAR_DEPTH);
	CTexture::setNullTexture(TS_DEFERRED_SELF_ILLUMINATION);

	// Activate el multi-render-target MRT
	const int nrender_targets = 4;
	ID3D11RenderTargetView* rts[nrender_targets] = {
	  rt_albedos->getRenderTargetView(),
	  rt_normals->getRenderTargetView(),
	  rt_depth->getRenderTargetView(),
	  rt_self_illum->getRenderTargetView(),
	};

	// We use our 3 rt's and the Zbuffer of the backbuffer
	Render.ctx->OMSetRenderTargets(nrender_targets, rts, Render.depthStencilView);
	rt_albedos->activateViewport();   // Any rt will do...

	// Clear output buffers, some can be removed if we intend to fill all the screen
	// with new data.
	rt_albedos->clear(VEC4(1, 0, 0, 1));
	rt_normals->clear(VEC4(0, 0, 1, 1));
	rt_depth->clear(VEC4(1, 1, 1, 1));
	rt_self_illum->clear(VEC4(0, 0, 0, 1));

	// Clear ZBuffer with the value 1.0 (far)
    Render.ctx->ClearDepthStencilView(Render.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the solid objects that output to the G-Buffer
	CRenderManager::get().renderCategory("gbuffer");

	// Disable rendering to all render targets.
	ID3D11RenderTargetView* rt_nulls[nrender_targets];
	for (int i = 0; i < nrender_targets; ++i) rt_nulls[i] = nullptr;
	Render.ctx->OMSetRenderTargets(nrender_targets, rt_nulls, nullptr);

	// Activate the gbuffer textures to other shaders
	rt_albedos->activate(TS_DEFERRED_ALBEDOS);
	rt_normals->activate(TS_DEFERRED_NORMALS);
	rt_self_illum->activate(TS_DEFERRED_SELF_ILLUMINATION);
	rt_depth->activate(TS_DEFERRED_LINEAR_DEPTH);
}

// -----------------------------------------------------------------
bool CDeferredRenderer::create(int xres, int yres) {

	rt_albedos = new CRenderToTexture;
	if (!rt_albedos->createRT("g_albedos.dds", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM))
		return false;

	rt_normals = new CRenderToTexture;
	if (!rt_normals->createRT("g_normals.dds", xres, yres, DXGI_FORMAT_R16G16B16A16_UNORM))
		return false;

	rt_self_illum = new CRenderToTexture;
	if (!rt_self_illum->createRT("g_self_illum.dds", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM))
		return false;

	rt_depth = new CRenderToTexture;
	if (!rt_depth->createRT("g_depths.dds", xres, yres, DXGI_FORMAT_R32_FLOAT))
		return false;

	rt_acc_light = new CRenderToTexture;
	if (!rt_acc_light->createRT("acc_light.dds", xres, yres, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN, true))
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

    CRenderManager::get().renderCategory("hologram");
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

		// subir las contantes de la posicion/dir
		// activar el shadow map...
		c->activate();

		setWorldTransform(c->getWorld());

		// mandar a pintar una geometria que refleje los pixeles que potencialmente
		// puede iluminar esta luz.... El Frustum solido
		mesh->render();
	});
}


// -------------------------------------------------------------------------
void CDeferredRenderer::renderDirectionalLights() {

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

	//Get the culling bits from the main camera 
	CEntity* e_camera = getEntityByName("main_camera");
	const TCompCulling* culling = nullptr;
	if (e_camera)
		culling = e_camera->get<TCompCulling>();
	const TCompCulling::TCullingBits* culling_bits = culling ? &culling->bits : nullptr;

	// Activate tech for the light dir 
	auto* tech = Resources.get("pbr_spot_lights.tech")->as<CRenderTechnique>();
	tech->activate();

	// All light directional use the same mesh
	auto* mesh = Resources.get("data/meshes/UnitCone.mesh")->as<CRenderMesh>();
	mesh->activate();

	// Para todas las luces... pintala
	getObjectManager<TCompLightSpot>()->forEach([&](TCompLightSpot* c) {


		// Do the culling 
		if (culling_bits) {
			CHandle h = c;
			CEntity* e = h.getOwner();
			std::string name = e->getName();
			TCompAbsAABB* aabb = e->get<TCompAbsAABB>();

			if (aabb) {
				CHandle h = aabb;
				auto idx = h.getExternalIndex();
				if (!culling_bits->test(idx)) {
					//Doing the culling, not painting the lights 
					//dbg("Not painting the light %s \n \n", name.c_str()); 
				}
				else {
					// subir las contantes de la posicion/dir 
					// activar el shadow map... 
					c->activate();

					setWorldTransform(c->getWorld());

					// mandar a pintar una geometria que refleje los pixeles que potencialmente 
					// puede iluminar esta luz.... El Frustum solido 
					mesh->render();
				}
			}
		}
	});
}

// Optimize this in the next milestone, use CS
// -------------------------------------------------------------------------
void CDeferredRenderer::renderVolumes() {

    //EngineInstancing.clearInstance("data/meshes/quad_volume.instanced_mesh");
    CTraceScoped gpu_scope("renderSpotLights");

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

	if (!h_camera.isValid()) return;

	CEntity* e_camera = h_camera;
	assert(e_camera);
	TCompRenderAO* comp_ao = e_camera->get<TCompRenderAO>();
	if (!comp_ao) {
		// As there is no comp AO, use a white texture as substitute
		const CTexture* white_texture = Resources.get("data/textures/white.dds")->as<CTexture>();
		white_texture->activate(TS_DEFERRED_AO);
		return;
	}
	// As we are going to update the RenderTarget AO
	// it can NOT be active as a texture while updating it.
	CTexture::setNullTexture(TS_DEFERRED_AO);
	auto ao = comp_ao->compute(rt_depth);
	// Activate the updated AO texture so everybody else can use it
	// Like the AccLight (Ambient pass or the debugger)
	ao->activate(TS_DEFERRED_AO);
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
    Render.ctx->OMSetRenderTargets(nrender_targets, rts, Render.depthStencilView);
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

// --------------------------------------
void CDeferredRenderer::render(CRenderToTexture* rt_destination, CHandle h_camera) {

	assert(rt_destination);
	renderGBuffer();
    renderGBufferDecals();
	renderAO(h_camera);

	// Do the same with the acc light
	CTexture::setNullTexture(TS_DEFERRED_ACC_LIGHTS);
	renderAccLight();
    //renderVolumes();

	// Now dump contents to the destination buffer.
	rt_destination->activateRT();
	rt_acc_light->activate(TS_DEFERRED_ACC_LIGHTS);

	// Combine the results
	renderFullScreenQuad("gbuffer_resolve.tech", nullptr);
}

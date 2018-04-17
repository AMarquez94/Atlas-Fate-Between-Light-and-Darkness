#include "mcv_platform.h"
#include "deferred_renderer.h"
#include "render_manager.h"
#include "render_utils.h"
#include "render_objects.h"
#include "resources/resources_manager.h"
#include "components/lighting/comp_light_dir.h"
#include "components/lighting/comp_light_point.h"
#include "components/lighting/comp_light_spot.h"
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
	rt_self_illum->clear(VEC4(1, 0, 0, 1));

	// Clear ZBuffer with the value 1.0 (far)
	Render.ctx->ClearDepthStencilView(Render.depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

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
}

// -----------------------------------------------------------------
bool CDeferredRenderer::create(int xres, int yres) {

	rt_albedos = new CRenderToTexture;
	if (!rt_albedos->createRT("g_albedos.dds", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM))
		return false;

	rt_normals = new CRenderToTexture;
	if (!rt_normals->createRT("g_normals.dds", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM))
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
	renderDirectionalLights();
	renderSkyBox();
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
	auto* mesh = Resources.get("data/meshes/UnitFrustum.mesh")->as<CRenderMesh>();
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

		// subir las contantes de la posicion/dir
		// activar el shadow map...
		c->activate();

		setWorldTransform(c->getWorld());

		// mandar a pintar una geometria que refleje los pixeles que potencialmente
		// puede iluminar esta luz.... El Frustum solido
		mesh->render();
	});
}

// --------------------------------------
void CDeferredRenderer::render(CRenderToTexture* rt_destination) {

	assert(rt_destination);
	renderGBuffer();

	// Do the same with the acc light
	CTexture::setNullTexture(TS_DEFERRED_ACC_LIGHTS);
	renderAccLight();

	// Now dump contents to the destination buffer.
	rt_destination->activateRT();
	rt_acc_light->activate(TS_DEFERRED_ACC_LIGHTS);

	// Combine the results
	renderFullScreenQuad("gbuffer_resolve.tech", nullptr);
}

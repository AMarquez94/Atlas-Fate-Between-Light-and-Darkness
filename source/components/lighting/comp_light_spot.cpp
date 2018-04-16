#include "mcv_platform.h"
#include "comp_light_spot.h"
#include "../comp_transform.h"
#include "render/render_objects.h"    // cb_light
#include "render/texture/texture.h"
#include "render/texture/render_to_texture.h" 
#include "render/render_manager.h" 
#include "render/render_utils.h"
#include "render/gpu_trace.h"
#include "ctes.h"                     // texture slots

DECL_OBJ_MANAGER("light_spot", TCompLightSpot);

void TCompLightSpot::debugInMenu() {
	ImGui::ColorEdit3("Color", &color.x);
	ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.f, 10.f);
	ImGui::DragFloat("Angle", &angle, 0.5f, 0.f, 180.f);
	ImGui::DragFloat("Range", &range, 0.5f, 0.f, 120.f);
}

void TCompLightSpot::renderDebug() {

	// Render a wire sphere
	auto mesh = Resources.get("data/meshes/UnitCone.mesh")->as<CRenderMesh>();
	renderMesh(mesh, getWorld(), VEC4(1, 1, 1, 1));
}

void TCompLightSpot::load(const json& j, TEntityParseContext& ctx) {

	isEnabled = true;
	TCompCamera::load(j, ctx);

	intensity = j.value("intensity", 1.0f);
	color = loadVEC4(j["color"]);

	casts_shadows = j.value("shadows", true);
	angle = j.value("angle", 45.f);
	range = j.value("range", 10.f);

	if (j.count("projector")) {
		std::string projector_name = j.value("projector", "");
		projector = Resources.get(projector_name)->as<CTexture>();
	}

	// Check if we need to allocate a shadow map
	casts_shadows = j.value("casts_shadows", false);
	if (casts_shadows) {
		shadows_step = j.value("shadows_step", shadows_step);
		shadows_resolution = j.value("shadows_resolution", shadows_resolution);
		auto shadowmap_fmt = readFormat(j, "shadows_fmt");
		assert(shadows_resolution > 0);
		shadows_rt = new CRenderToTexture;
		// Make a unique name to have the Resource Manager happy with the unique names for each resource
		char my_name[64];
		sprintf(my_name, "shadow_map_%08x", CHandle(this).asUnsigned());
		bool is_ok = shadows_rt->createRT(my_name, shadows_resolution, shadows_resolution, DXGI_FORMAT_UNKNOWN, shadowmap_fmt);
		assert(is_ok);
	}

	shadows_enabled = casts_shadows;
}

MAT44 TCompLightSpot::getWorld() {

	TCompTransform* c = get<TCompTransform>();
	if (!c)
		return MAT44::Identity;

	float new_scale = tan(-angle * .5f) * range;
	dbg("angle value %f\n", new_scale);
	return MAT44::CreateScale(VEC3(new_scale, new_scale, range)) * c->asMatrix();
}

void TCompLightSpot::update(float dt) {

	TCompTransform * c = get<TCompTransform>();
	if (!c)
		return;

	this->lookAt(c->getPosition(), c->getPosition() + c->getFront(), c->getUp());
}

void TCompLightSpot::registerMsgs() {

	DECL_MSG(TCompLightSpot, TMsgEntityCreated, onCreate);
	DECL_MSG(TCompLightSpot, TMsgEntityDestroyed, onDestroy);
}

void TCompLightSpot::onCreate(const TMsgEntityCreated& msg) {

	//EnginePhysics.createActor(*this);

}

void TCompLightSpot::onDestroy(const TMsgEntityDestroyed & msg) {

}

void TCompLightSpot::activate() {

	TCompTransform* c = get<TCompTransform>();
	if (!c)
		return;

	cb_light.light_color = color;
	cb_light.light_intensity = intensity;
	cb_light.light_pos = c->getPosition();
	cb_light.light_radius = 1;
	cb_light.light_view_proj_offset = MAT44::Identity;
	cb_light.use_projector = 0;
	cb_light.updateGPU();
}

// ------------------------------------------------------
void TCompLightSpot::generateShadowMap() {

	//if (!shadows_rt || !shadows_enabled)
	//	return;

	//// In this slot is where we activate the render targets that we are going
	//// to update now. You can't be active as texture and render target at the
	//// same time
	//CTexture::setNullTexture(TS_LIGHT_SHADOW_MAP);

	//CTraceScoped gpu_scope(shadows_rt->getName().c_str());
	//shadows_rt->activateRT();

	//{
	//	PROFILE_FUNCTION("Clear&SetCommonCtes");
	//	shadows_rt->clearZ();
	//	// We are going to render the scene from the light position & orientation
	//	activateCamera(*this, shadows_rt->getWidth(), shadows_rt->getHeight());
	//}

	//CRenderManager::get().renderCategory("shadows");
}



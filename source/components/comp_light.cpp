#include "mcv_platform.h"
#include "comp_light.h"
#include "comp_transform.h"
#include "entity/entity_parser.h"
#include "render/render_objects.h"    // cb_light
#include "render/texture/texture.h"
#include "render/texture/texture_slots.h"

DECL_OBJ_MANAGER("light", TCompLight);

void TCompLight::debugInMenu() {
	TCompCamera::debugInMenu();
}

void TCompLight::renderDebug() {

	TCompCamera::renderDebug();
	ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.f, 10.f);
	ImGui::ColorEdit3("Color", &color.x);
}

void TCompLight::load(const json& j, TEntityParseContext& ctx) {

	type = j.value("type", "default"); // Might move this out of here ?
	intensity = j.value("intensity", 1.0f);
	color = loadVEC4(j["color"]);

	cast_shadows = j.value("shadows", true);
	angle = j.value("angle", 45.f);
	range = j.value("range", 10.f);

	if (j.count("projector")) {
		std::string projector_name = j.value("projector", "");
		projector = Resources.get(projector_name)->as<CTexture>();
	}
}


void TCompLight::update(float dt) {
	// Can't use the TCompCamera::update because inside it calls 
		  // get<TCompTransform> which tries to convert 'this' to an instance
		  // of TCompCamera, but will fail because we are a CompLightDir
	TCompTransform * c = get<TCompTransform>();
	if (!c)
		return;
	this->lookAt(c->getPosition(), c->getPosition() + c->getFront(), c->getUp());
}

void TCompLight::registerMsgs() {

	DECL_MSG(TCompLight, TMsgEntityCreated, onCreate);
	DECL_MSG(TCompLight, TMsgEntityDestroyed, onDestroy);
}


void TCompLight::onCreate(const TMsgEntityCreated& msg) {

	//EnginePhysics.createActor(*this);

}

void TCompLight::onDestroy(const TMsgEntityDestroyed & msg) {

}

void TCompLight::activate() {

	TCompTransform * c = get<TCompTransform>();
	if (!c)
		return;

	if (projector)
		projector->activate(TS_LIGHT_PROJECTOR);

	// To avoid converting the range -1..1 to 0..1 in the shader
	// we concatenate the view_proj with a matrix to apply this offset
	MAT44 mtx_offset = MAT44::CreateScale(VEC3(0.5f, -0.5f, 1.0f))
					 * MAT44::CreateTranslation(VEC3(0.5f, 0.5f, 0.0f));

	cb_light.light_color = color;
	cb_light.light_intensity = intensity;
	cb_light.light_pos = c->getPosition();
	cb_light.light_view_proj_offset = getViewProjection() * mtx_offset;
	cb_light.updateGPU();
}
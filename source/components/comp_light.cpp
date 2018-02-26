#include "mcv_platform.h"
#include "comp_light.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("light", TCompLight);

void TCompLight::debugInMenu() {

}

void TCompLight::load(const json& j, TEntityParseContext& ctx) {

	intensity = j.value("intensity", 1.0f);
	color = loadVEC3(j["color"]);
	type = j.value("type", "default");
	cast_shadows = j.value("shadows", true);
	angle = j.value("angle", 45.f);
	range = j.value("range", 10.f);
}


void TCompLight::update(float dt) {

}

void TCompLight::registerMsgs() {

	DECL_MSG(TCompLight, TMsgEntityCreated, onCreate);
	DECL_MSG(TCompLight, TMsgEntityDestroyed, onDestroy);
}


void TCompLight::onCreate(const TMsgEntityCreated& msg) {

	//EnginePhysics.createActor(*this);

}

void TCompLight::onDestroy(const TMsgEntityDestroyed & msg){

}

void TCompLight::BuildTrigger(void){

}
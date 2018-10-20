#include "mcv_platform.h"
#include "comp_omni_fade_out.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/lighting/comp_light_point.h"

DECL_OBJ_MANAGER("omni_fade_out", TCompOmniFadeOut);

void TCompOmniFadeOut::debugInMenu() {

}

void TCompOmniFadeOut::load(const json& j, TEntityParseContext& ctx) {

}

void TCompOmniFadeOut::update(float dt) {

}

void TCompOmniFadeOut::registerMsgs() {

	DECL_MSG(TCompOmniFadeOut, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompOmniFadeOut, TMsgOmniFadeOut, onMsgFadeOut);
}

void TCompOmniFadeOut::onMsgEntityCreated(const TMsgEntityCreated& msg) {

}

void TCompOmniFadeOut::onMsgFadeOut(const TMsgOmniFadeOut& msg) {
	CEntity *e = CHandle(this).getOwner();
	TCompLightPoint *light_point = e->get<TCompLightPoint>();
	EngineLerp.lerpElement(&light_point->intensity, 0.0f,3.f,0.0f);
}
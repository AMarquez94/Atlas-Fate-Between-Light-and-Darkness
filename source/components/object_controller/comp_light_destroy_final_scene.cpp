#include "mcv_platform.h"
#include "comp_light_destroy_final_scene.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/lighting/comp_light_point.h"

DECL_OBJ_MANAGER("light_destroy_final_scene", TCompLightDestroyFinalScene);

void TCompLightDestroyFinalScene::debugInMenu() {

}

void TCompLightDestroyFinalScene::load(const json& j, TEntityParseContext& ctx) {
	time_to_destroy = j.value("time_to_destroy", 0.0);
}

void TCompLightDestroyFinalScene::update(float dt) {

	if (this != nullptr && active) {
		if (current_time >= time_to_destroy) {
			CHandle(this).getOwner().destroy();
		}
		current_time += dt;
	}
}

void TCompLightDestroyFinalScene::registerMsgs() {
	DECL_MSG(TCompLightDestroyFinalScene, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompLightDestroyFinalScene, TMsgEmisiveCapsuleState, onMsgEmisiveCapsuleState);

}

void TCompLightDestroyFinalScene::onMsgEntityCreated(const TMsgEntityCreated& msg) {


}

void TCompLightDestroyFinalScene::onMsgEmisiveCapsuleState(const TMsgEmisiveCapsuleState& msg) {
	active = msg.enable;
}
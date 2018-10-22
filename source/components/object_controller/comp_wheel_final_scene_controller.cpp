#include "mcv_platform.h"
#include "comp_wheel_final_scene_controller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/lighting/comp_light_point.h"
#include "components/lighting/comp_light_spot.h"

DECL_OBJ_MANAGER("wheel_final_controller", TCompWheelFinalSceneController);

void TCompWheelFinalSceneController::debugInMenu() {

}

void TCompWheelFinalSceneController::load(const json& j, TEntityParseContext& ctx) {

}

void TCompWheelFinalSceneController::update(float dt) {
}

void TCompWheelFinalSceneController::registerMsgs() {
	DECL_MSG(TCompWheelFinalSceneController, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompWheelFinalSceneController, TMsgRotatorAccelerate, onMsgWheelRotating);

}

void TCompWheelFinalSceneController::onMsgEntityCreated(const TMsgEntityCreated& msg) {

}

void TCompWheelFinalSceneController::onMsgWheelRotating(const TMsgRotatorAccelerate& msg) {

	if (msg.speed != 0.0f) return;

	CEntity *aux_ent = CHandle(this).getOwner();
	TCompLightPoint *comp_light_point = aux_ent->get<TCompLightPoint>();
	if (comp_light_point != nullptr) {
		EngineLerp.lerpElement(&comp_light_point->intensity, msg.speed, msg.time_to_lerp, 0.0f);
	}
	else {
		TCompLightSpot *comp_light_spot = aux_ent->get<TCompLightSpot>();
		if (comp_light_spot != nullptr) {
			EngineLerp.lerpElement(&comp_light_spot->intensity, msg.speed, msg.time_to_lerp, 0.0f);
		}
	}
}
#include "mcv_platform.h"
#include "comp_rotator_accelerator_final_scene.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/lighting/comp_light_point.h"
#include "components/object_controller/comp_rotator.h"

DECL_OBJ_MANAGER("rotator_accelerator_final_scene", TCompRotatorAcceleratorFinalScene);

void TCompRotatorAcceleratorFinalScene::debugInMenu() {

}

void TCompRotatorAcceleratorFinalScene::load(const json& j, TEntityParseContext& ctx) {

}

void TCompRotatorAcceleratorFinalScene::update(float dt) {

	
}

void TCompRotatorAcceleratorFinalScene::registerMsgs() {
	DECL_MSG(TCompRotatorAcceleratorFinalScene, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompRotatorAcceleratorFinalScene, TMsgRotatorAccelerate, onMsgStartAcceleration);

}

void TCompRotatorAcceleratorFinalScene::onMsgEntityCreated(const TMsgEntityCreated& msg) {
	CHandle own_handler = CHandle(this).getOwner();
	CEntity* own_e = own_handler;
	rotator_handle = own_e->get<TCompRotator>();
}

void TCompRotatorAcceleratorFinalScene::onMsgStartAcceleration(const TMsgRotatorAccelerate& msg) {
	TCompRotator *comp_rotator = rotator_handle;
	EngineLerp.lerpElement(&comp_rotator->_speed, msg.speed, msg.time_to_lerp,msg.time_to_start);
}
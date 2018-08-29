#include "mcv_platform.h"
#include "comp_animated_object_controller.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("animated_object_controller", TCompAnimatedObjController);

void TCompAnimatedObjController::registerMsgs() {

}

void TCompAnimatedObjController::debugInMenu() {

}

void TCompAnimatedObjController::load(const json& j, TEntityParseContext& ctx) {
	//Pillar el nom dels objectes que es guarden la merda del animated objects
}

void TCompAnimatedObjController::update(float dt) {
	//Crec que no s'utilitzara, pero nose lol jajaja omg que nup
}


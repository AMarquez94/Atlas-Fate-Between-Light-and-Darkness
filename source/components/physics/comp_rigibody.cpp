#include "mcv_platform.h"
#include "comp_rigibody.h"
#include "../comp_transform.h"
#include "../player_controller/comp_player_controller.h"

DECL_OBJ_MANAGER("rigibody", TCompRigidbody);

void TCompRigidbody::debugInMenu() {

}

void TCompRigidbody::load(const json& j, TEntityParseContext& ctx) {


}

void TCompRigidbody::update(float dt) {


}

/* Collider/Trigger messages */
void TCompRigidbody::registerMsgs() {

	DECL_MSG(TCompRigidbody, TMsgEntityCreated, onCreate);
}

void TCompRigidbody::onCreate(const TMsgEntityCreated& msg) {

	//CEngine::get().getPhysics().createActor(*this);
}

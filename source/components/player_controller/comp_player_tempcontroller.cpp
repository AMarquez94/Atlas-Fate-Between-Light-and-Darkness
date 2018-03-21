#include "mcv_platform.h"
#include "comp_player_tempcontroller.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("player_tempcontroller", TCompTempPlayerController);

void TCompTempPlayerController::debugInMenu() {

}

void TCompTempPlayerController::load(const json& j, TEntityParseContext& ctx) {

}

void TCompTempPlayerController::update(float dt)
{

}

void TCompTempPlayerController::registerMsgs() {

	DECL_MSG(TCompTempPlayerController, TMsgAction, onAction);
}

void TCompTempPlayerController::onAction(const TMsgAction& msg)
{

}

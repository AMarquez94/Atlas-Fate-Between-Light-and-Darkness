#include "mcv_platform.h"
#include "comp_button.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("button", TCompButton);

void TCompButton::debugInMenu() {
}

void TCompButton::load(const json& j, TEntityParseContext& ctx) {

	_scriptName = j.value("script", "");

}

void TCompButton::onMsgButtonActivated(const TMsgButtonActivated& msg) {
	EngineLogic.execScript(_scriptName);
}

void TCompButton::registerMsgs()
{
	DECL_MSG(TCompButton, TMsgButtonActivated, onMsgButtonActivated);

}

void TCompButton::update(float dt)
{

}

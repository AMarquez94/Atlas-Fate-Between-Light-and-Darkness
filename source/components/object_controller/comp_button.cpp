#include "mcv_platform.h"
#include "comp_button.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("button", TCompButton);

void TCompButton::debugInMenu() {
	ImGui::Text("Script: %s", _scriptName.c_str());
}

void TCompButton::load(const json& j, TEntityParseContext& ctx) {

	_scriptName = j.value("script", "");
	_delay = j.value("delay", 0.0);

}

void TCompButton::onMsgButtonActivated(const TMsgButtonActivated& msg) {

	if (_delay > 0.0) {
		EngineLogic.execScriptDelayed(_scriptName, _delay);
	}
	else {
		EngineLogic.execScript(_scriptName);
	}
	
}

void TCompButton::registerMsgs()
{
	DECL_MSG(TCompButton, TMsgButtonActivated, onMsgButtonActivated);

}

void TCompButton::update(float dt)
{

}

#include "mcv_platform.h"
#include "comp_button.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("button", TCompButton);

void TCompButton::debugInMenu() {

	for (int i = 0; i < _scripts.size(); i++) {

		ImGui::Text("Script %i: Name: %s Delay: %f", i, _scripts[i].name.c_str(), _scripts[i].delay);

	}
}

void TCompButton::load(const json& j, TEntityParseContext& ctx) {

	if (j.count("scripts") > 0) {
		auto& j_scripts = j["scripts"];
		_scripts.clear();
		script s;
		for (auto it = j_scripts.begin(); it != j_scripts.end(); ++it) {

			assert(it.value().count("script") == 1);

			s.name = it.value().value("script", "");
			s.delay = it.value().value("delay", 0.0f);

			_scripts.push_back(s);

		}
	}

}

void TCompButton::onMsgButtonActivated(const TMsgButtonActivated& msg) {

	for (int i = 0; i < _scripts.size(); i++) {
		if (_scripts[i].delay > 0.0) {
			EngineLogic.execScriptDelayed(_scripts[i].name, _scripts[i].delay);
		}
		else {
			EngineLogic.execScript(_scripts[i].name);
		}

	}

}

void TCompButton::registerMsgs()
{
	DECL_MSG(TCompButton, TMsgButtonActivated, onMsgButtonActivated);

}

void TCompButton::update(float dt)
{

}

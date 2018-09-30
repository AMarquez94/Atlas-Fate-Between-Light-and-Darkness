#include "mcv_platform.h"
#include "comp_button.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"
#include "components/skeleton/comp_player_animation_placer.h"

DECL_OBJ_MANAGER("button", TCompButton);

void TCompButton::debugInMenu() {

    ImGui::Text("Script Name: %s\n", _script.c_str());
}

void TCompButton::load(const json& j, TEntityParseContext& ctx) {
    _script = j.value("script", "");
    canBePressed = j.value("canBePressed", true);
}

void TCompButton::onMsgButtonActivated(const TMsgButtonActivated& msg) {
    if (canBePressed) {
        EngineLogic.execScript(_script);
    }
}
void TCompButton::onMsgGroupCreated(const TMsgEntitiesGroupCreated& msg) {
	CEntity *e = CHandle(this).getOwner();
	//TCompPlayerAnimatorPlacer *comp_placer = e->get<TCompPlayerAnimatorPlacer>();
	//assert(comp_placer);
}


void TCompButton::registerMsgs()
{
    DECL_MSG(TCompButton, TMsgButtonActivated, onMsgButtonActivated);
	DECL_MSG(TCompButton, TMsgEntitiesGroupCreated, onMsgGroupCreated);
}

void TCompButton::update(float dt)
{

}

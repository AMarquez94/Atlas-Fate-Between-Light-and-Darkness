#include "mcv_platform.h"
#include "comp_button.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"
#include "components/skeleton/comp_player_animation_placer.h"
#include "components/lighting/comp_emission_controller.h"

DECL_OBJ_MANAGER("button", TCompButton);

void TCompButton::debugInMenu() {

    ImGui::Text("Script Name: %s\n", _script.c_str());
}

void TCompButton::load(const json& j, TEntityParseContext& ctx) {
    _script = j.value("script", "");
    canBePressed = j.value("canBePressed", true);
}

void TCompButton::onMsgEntityCreated(const TMsgEntityCreated& msg) {
    TCompEmissionController * my_emission = get <TCompEmissionController>();
    if (my_emission) {
        my_emission->blend(canBePressed ? VEC4(0.f, 1.f, 0.f, 1.f) : VEC4(1.f, 0.f, 0.f, 1.f), 0.1f);
    }
}

void TCompButton::onMsgButtonActivated(const TMsgButtonActivated& msg) {
    if (canBePressed) {
        EngineLogic.execScript(_script + "(" + CHandle(this).getOwner().asString() + ")");
    }
}
void TCompButton::onMsgGroupCreated(const TMsgEntitiesGroupCreated& msg) {
	CEntity *e = CHandle(this).getOwner();
}

void TCompButton::setCanBePressed(bool canBePressed) {
    this->canBePressed = canBePressed;
    TCompEmissionController * my_emission = get <TCompEmissionController>();
    if (my_emission) {
        my_emission->blend( canBePressed ? VEC4(0.f, 1.f, 0.f, 1.f) : VEC4(1.f, 0.f, 0.f, 1.f), 0.5f);
    }
}


void TCompButton::registerMsgs()
{
    DECL_MSG(TCompButton, TMsgButtonActivated, onMsgButtonActivated);
    DECL_MSG(TCompButton, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompButton, TMsgEntitiesGroupCreated, onMsgGroupCreated);
}

void TCompButton::update(float dt)
{

}

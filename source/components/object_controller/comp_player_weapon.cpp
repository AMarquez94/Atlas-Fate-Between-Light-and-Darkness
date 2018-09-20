#include "mcv_platform.h"
#include "comp_player_weapon.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("player_weapon", TCompPlayerWeapon);

void TCompPlayerWeapon::debugInMenu() {
    ImGui::Text("State %d", _currentState);
}

void TCompPlayerWeapon::load(const json& j, TEntityParseContext& ctx) {
    weapons_on_event = j.value("weapons_on_event", "");
    weapons_off_event = j.value("weapons_off_event", "");
    _currentState = EState::DEACTIVATED;
}

void TCompPlayerWeapon::onMsgWeaponsActivated(const TMsgWeaponsActivated& msg) {
    TCompAudio* my_audio = get<TCompAudio>();

    switch (_currentState) {
    case EState::DEACTIVATED:
        if (msg.activate) {
            if (weaponsOff.isValid()) {
                weaponsOff.stop();
            }
            weaponsOn = my_audio->playEvent(weapons_on_event);
            _currentState = EState::ACTIVATED;
        }
        break;
    case EState::ACTIVATED:
        if (!msg.activate) {
            weaponsOn.stop();
            weaponsOff = my_audio->playEvent(weapons_off_event);
            _currentState = EState::TURNING_OFF;
        }
        break;
    case EState::TURNING_OFF:
        if (msg.activate) {
            weaponsOff.stop();
            weaponsOn = my_audio->playEvent(weapons_on_event);
            _currentState = EState::ACTIVATED;
        }
        break;
    }
}

void TCompPlayerWeapon::registerMsgs()
{
    DECL_MSG(TCompPlayerWeapon, TMsgWeaponsActivated, onMsgWeaponsActivated);
}

void TCompPlayerWeapon::update(float dt)
{
    switch (_currentState) {
    case EState::TURNING_OFF:
        if (!weaponsOff.isValid()) {
            _currentState = EState::DEACTIVATED;
        }
        break;
    }
}

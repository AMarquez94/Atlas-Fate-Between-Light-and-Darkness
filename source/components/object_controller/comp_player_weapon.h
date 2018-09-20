#pragma once

#include "components/comp_base.h"

class TCompPlayerWeapon : public TCompBase
{
    DECL_SIBLING_ACCESS();

public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    static void registerMsgs();

private:

    enum EState { DEACTIVATED = 0, ACTIVATED, TURNING_OFF };

    void onMsgWeaponsActivated(const TMsgWeaponsActivated& msg);
    SoundEvent currentSound;
    SoundEvent weaponsOn;
    SoundEvent weaponsOff;
    SoundEvent weaponsHum;

    std::string weapons_on_event;
    std::string weapons_off_event;

    EState _currentState;
};


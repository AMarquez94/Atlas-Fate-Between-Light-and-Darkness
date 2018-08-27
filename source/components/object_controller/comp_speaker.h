#pragma once

#include "components/comp_base.h"
#include "sound/soundEvent.h"

struct TCompSpeaker : public TCompBase {

private:

    DECL_SIBLING_ACCESS();

    std::string soundEventName;
    SoundEvent soundEvent;

    void onMsgEntityCreated(const TMsgEntityCreated& msg);

public:

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    static void registerMsgs();
};

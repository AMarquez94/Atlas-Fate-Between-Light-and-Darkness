#pragma once

#include "comp_base.h"
#include "sound/soundEvent.h"

struct TCompAudio : public TCompBase {

private:

    DECL_SIBLING_ACCESS();

    std::vector<SoundEvent> my2DEvents;
    std::vector<SoundEvent> my3DEvents;

public:

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    static void registerMsgs();

    SoundEvent playEvent(const std::string& name);
    void stopAllEvents();

};

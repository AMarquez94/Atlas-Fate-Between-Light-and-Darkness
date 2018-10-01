#pragma once

#include "comp_base.h"
#include "sound/soundEvent.h"

struct TCompAudio : public TCompBase {

private:

    struct StartingSoundEvents {
        std::string name;
        bool relativeToPlayer;
    };

    DECL_SIBLING_ACCESS();

    std::vector<SoundEvent> my2DEvents;
    std::vector<SoundEvent> my3DEvents;

    std::vector<StartingSoundEvents> startingSoundEvents;

    void onAnimationAudioCallback(const TMsgAnimationAudioCallback& msg);
    void onStopAudioComponent(const TMsgStopAudioComponent& msg);
    void onSceneCreated(const TMsgSceneCreated& msg);


    void stopAudioComponent();

public:

    ~TCompAudio();

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    static void registerMsgs();

    SoundEvent playEvent(const std::string& name, bool relativeToPlayer = true);
    void stopAllEvents();
};

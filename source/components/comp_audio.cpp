#include "mcv_platform.h"
#include "comp_audio.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("audio", TCompAudio);

TCompAudio::~TCompAudio() {
    stopAudioComponent();
}

void TCompAudio::onAnimationAudioCallback(const TMsgAnimationAudioCallback & msg)
{
    playEvent(msg.audioName, msg.isRelativeToPlayer);
}

void TCompAudio::onStopAudioComponent(const TMsgStopAudioComponent & msg)
{
    stopAudioComponent();
}

void TCompAudio::stopAudioComponent()
{
    for (auto audio : my2DEvents) {
        if (audio.isValid()) {
            audio.stop();
        }
    }

    for (auto audio : my3DEvents) {
        if (audio.isValid()) {
            audio.stop();
        }
    }
}

void TCompAudio::debugInMenu()
{
    for (auto audio : my3DEvents) {
        if (audio.isValid()) {
            CTransform t = audio.get3DAttributes();
            ImGui::Text("Position: %f %f %f", t.getPosition().x, t.getPosition().y, t.getPosition().z);
        }
    }
}

void TCompAudio::load(const json & j, TEntityParseContext & ctx)
{
    //if (j.count("onStart") > 0) {
    //    auto& j_onStart = j["onStart"];
    //    for (auto it = j_onStart.begin(); it != j_onStart.end(); ++it) {
    //        //TODO: Test
    //        playEvent(it.value().value("eventName", ""), it.value().value("relativeToPlayer", true));
    //    }
    //}
}

void TCompAudio::update(float dt)
{
    // Remove invalid 2D events
    auto iter = my2DEvents.begin();
    while (iter != my2DEvents.end()) {
        if (!iter->isValid()) {
            iter = my2DEvents.erase(iter);
        }
        else {
            iter++;
        }
    }

    // Remove invalid 3D events
    iter = my3DEvents.begin();
    while (iter != my3DEvents.end()) {
        if (!iter->isValid()) {
            iter = my3DEvents.erase(iter);
        }
        else {
            iter++;
        }
    }

    // Update position of the valid
    TCompTransform* mypos = get<TCompTransform>();
    for (auto& event : my3DEvents) {
        if (event.isValid()) {
            if (CHandle(this).getOwner().isValid()) {
                if (event.isRelativeToCameraOnly()) {
                    event.set3DAttributes(*mypos);
                }
                else {
                    event.set3DAttributes(EngineSound.getVirtual3DAttributes(*mypos));
                }
            }
        }
    }
}

void TCompAudio::registerMsgs()
{
    DECL_MSG(TCompAudio, TMsgAnimationAudioCallback, onAnimationAudioCallback);
    DECL_MSG(TCompAudio, TMsgStopAudioComponent, onStopAudioComponent);
}

SoundEvent TCompAudio::playEvent(const std::string & name, bool relativeToPlayer)
{
    SoundEvent e = EngineSound.playEvent(name);
    if (e.is3D()) {
        TCompTransform* mypos = get<TCompTransform>();
        e.setIsRelativeToCameraOnly(!relativeToPlayer);
        if (relativeToPlayer) {
            e.set3DAttributes(EngineSound.getVirtual3DAttributes(*mypos));
        }
        else {
            e.set3DAttributes(*mypos);
        }
        my3DEvents.emplace_back(e);
    }
    else {
        my2DEvents.emplace_back(e);
    }
    return e;
}

void TCompAudio::stopAllEvents()
{
    for (auto& e : my2DEvents) {
        e.stop();
    }
    for (auto&e : my3DEvents) {
        e.stop();
    }

    my2DEvents.clear();
    my3DEvents.clear();
}
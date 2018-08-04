#include "mcv_platform.h"
#include "comp_audio.h"

DECL_OBJ_MANAGER("audio", TCompAudio);

void TCompAudio::debugInMenu()
{
}

void TCompAudio::load(const json & j, TEntityParseContext & ctx)
{
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

    TCompTransform* mypos = get<TCompTransform>();

    // Remove invalid 3D events (and update its position)
    iter = my3DEvents.begin();
    while (iter != my3DEvents.end()) {
        if (!iter->isValid()) {
            iter = my3DEvents.erase(iter);
        }
        else {
            //update my pos
            iter->set3DAttributes(*mypos);  //TODO: test if this works
            iter++;
        }
    }
}

void TCompAudio::registerMsgs()
{
}

SoundEvent TCompAudio::playEvent(const std::string & name)
{
    SoundEvent e = EngineSound.playEvent(name);
    if (e.is3D()) {
        my3DEvents.emplace_back(e);
        TCompTransform* mypos = get<TCompTransform>();
        e.set3DAttributes(*mypos);
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

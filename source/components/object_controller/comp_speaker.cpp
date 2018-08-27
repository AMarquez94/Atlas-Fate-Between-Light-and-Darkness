#include "mcv_platform.h"
#include "comp_speaker.h"
#include "components/comp_audio.h"

DECL_OBJ_MANAGER("speaker", TCompSpeaker);

void TCompSpeaker::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
    TCompAudio* audio = get<TCompAudio>();
    soundEvent = audio->playEvent(soundEventName);
}

void TCompSpeaker::debugInMenu()
{

}

void TCompSpeaker::load(const json & j, TEntityParseContext & ctx)
{
    soundEventName = j.value("soundEventName", "event:/Sounds/Debug/Helicopter");
}

void TCompSpeaker::update(float dt)
{

}

void TCompSpeaker::registerMsgs()
{
    DECL_MSG(TCompSpeaker, TMsgEntityCreated, onMsgEntityCreated);
}
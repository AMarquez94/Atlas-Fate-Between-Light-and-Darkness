#include "mcv_platform.h"
#include "comp_audio.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("audio", TCompAudio);

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
            if (event.isRelativeToCameraOnly()) {
                event.set3DAttributes(*mypos);
            }
            else {
                event.set3DAttributes(getVirtual3DAttributes());
            }
        }
    }
}

void TCompAudio::registerMsgs()
{
}

SoundEvent TCompAudio::playEvent(const std::string & name, bool relativeToPlayer)
{
    SoundEvent e = EngineSound.playEvent(name);
    if (e.is3D()) {
        TCompTransform* mypos = get<TCompTransform>();
        e.setIsRelativeToCameraOnly(!relativeToPlayer);
        if (relativeToPlayer) {
            e.set3DAttributes(getVirtual3DAttributes());
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

CTransform TCompAudio::getVirtual3DAttributes()
{
    TCompTransform* mypos = get<TCompTransform>();

    CHandle h_listener = EngineSound.getListener();
    if (h_listener.isValid()) {
        CEntity* e_listener = EngineSound.getListener();
        TCompTags* listener_tags = e_listener->get<TCompTags>();

        if (listener_tags && listener_tags->hasTag(getID("main_camera"))) {
            CEntity* player = getEntityByName("The Player");
            TCompTransform* ppos = player->get<TCompTransform>();
            TCompTransform* listenerPos = e_listener->get<TCompTransform>();

            float distance = VEC3::Distance(ppos->getPosition(), mypos->getPosition());
            VEC3 direction = (mypos->getPosition() - listenerPos->getPosition()).Normalized();
            VEC3 virtualPos = listenerPos->getPosition() + direction * distance;

            CTransform newTransform;
            newTransform.setPosition(virtualPos);
            newTransform.setRotation(mypos->getRotation());
            newTransform.setScale(mypos->getScale());
            return newTransform;
        }
        else {
            return *mypos;
        }
    }
    else {
        return *mypos;
    }
}

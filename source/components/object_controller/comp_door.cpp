#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_door.h"
#include "components/comp_transform.h"
#include "components/physics/comp_collider.h"
#include "render/render_objects.h"
#include "physics/physics_collider.h"
#include "components/comp_animated_object_controller.h"

DECL_OBJ_MANAGER("door", TCompDoor);

void TCompDoor::onScenePaused(const TMsgScenePaused & msg)
{
    paused = msg.isPaused;
}

void TCompDoor::debugInMenu() {

}

void TCompDoor::load(const json& j, TEntityParseContext& ctx) {
    opening_anim = j.value("open_anim", "");
    closing_anim = j.value("close_anim", "");
    loading_anim = j.value("load_anim", "");
    time_to_open = j.value("time_to_open", 2.f);
    time_to_close = j.value("time_to_close", 3.f);
    open_soundevent = j.value("open_soundevent", "");
    close_soundevent = j.value("close_soundevent", "");
}

void TCompDoor::registerMsgs() {
    DECL_MSG(TCompDoor, TMsgScenePaused, onScenePaused);
}

void TCompDoor::update(float dt) {

    if (!CHandle(this).getOwner().isValid())
        return;

    if (!paused) {
        switch (state) {
        case EDoorState::OPENING:
            timer += dt;
            if (timer > time_to_open) {
                /* Collider open */
                TCompCollider* my_col = get<TCompCollider>();
                if (my_col) {
                    my_col->setGroupAndMask("ignore", "player");
                }
                state = EDoorState::OPENED;
                EngineLogic.execScript(opened_script);
                timer = 0.f;
            }
            break;
        case EDoorState::CLOSING:
            timer += dt;
            if (timer > time_to_close) {
                /* Collider close */
                state = EDoorState::CLOSED;
                EngineLogic.execScript(closed_script);
                timer = 0.f;
            }
            break;
        default:

            break;
        }

    }
}

void TCompDoor::open() {
    TCompAnimatedObjController* anim = get<TCompAnimatedObjController>();
    anim->playAnimation(opening_anim);
    TCompAudio* my_audio = get<TCompAudio>();
    if (my_audio) {
        my_audio->playEvent(open_soundevent);
    }
    state = EDoorState::OPENING;
    timer = 0.f;
}

void TCompDoor::close() {
    TCompAnimatedObjController* anim = get<TCompAnimatedObjController>();
    anim->playAnimation(closing_anim);
    state = EDoorState::CLOSING;
    TCompCollider* my_col = get<TCompCollider>();
    if (my_col) {
        my_col->setGroupAndMask("all", "all");
    }
    TCompAudio* my_audio = get<TCompAudio>();
    if (my_audio) {
        my_audio->playEvent(close_soundevent);
    }
    timer = 0.f;
}

void TCompDoor::load() {
    //TODO
    TCompAnimatedObjController* anim = get<TCompAnimatedObjController>();
    timer = 0.f;
}

void TCompDoor::setOpenedScript(const std::string& script) {
    opened_script = script;
}

void TCompDoor::setClosedScript(const std::string& script) {
    closed_script = script;
}
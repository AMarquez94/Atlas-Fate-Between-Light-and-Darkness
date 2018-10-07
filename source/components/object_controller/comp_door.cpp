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
    time_to_open = j.value("time_to_open", 1.f);
    time_to_close = j.value("time_to_close", 1.f);
}

void TCompDoor::registerMsgs() {
    DECL_MSG(TCompDoor, TMsgScenePaused, onScenePaused);
}

void TCompDoor::update(float dt) {
    if (!paused) {
        switch (state) {
        case EDoorState::OPENING:
            timer += dt;
            if (timer > time_to_open) {
                /* Collider open */
                TCompCollider* my_col = get<TCompCollider>();
                dbg("Puerta abierta\n");
                my_col->setGroupAndMask("ignore", "player");
                state = EDoorState::OPENED;
                timer = 0.f;
            }
            break;
        case EDoorState::CLOSING:
            timer += dt;
            if (timer > time_to_close) {
                /* Collider close */
                TCompCollider* my_col = get<TCompCollider>();
                dbg("Puerta cerrada\n");
                my_col->setGroupAndMask("all", "all");
                state = EDoorState::CLOSED;
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
    state = EDoorState::OPENING;
    timer = 0.f;
}

void TCompDoor::close() {
    TCompAnimatedObjController* anim = get<TCompAnimatedObjController>();
    anim->playAnimation(closing_anim);
    state = EDoorState::CLOSING;
    timer = 0.f;
}

void TCompDoor::loading() {
    //TODO
    TCompAnimatedObjController* anim = get<TCompAnimatedObjController>();
    timer = 0.f;
}
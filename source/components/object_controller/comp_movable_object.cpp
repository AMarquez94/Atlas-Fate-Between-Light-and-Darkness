#include "mcv_platform.h"
#include "comp_movable_object.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "render/render_objects.h"
#include "components/physics/comp_rigidbody.h"

DECL_OBJ_MANAGER("movable_object", TCompMovableObject);

void TCompMovableObject::onObjectBeingMoved(const TMsgObjectBeingMoved & msg)
{
    isBeingMoved = msg.isBeingMoved;
    hEntityMovingMe = msg.hMovingObject;
    direction = msg.direction;
    speed = msg.speed;
}

void TCompMovableObject::debugInMenu() {
}

void TCompMovableObject::load(const json& j, TEntityParseContext& ctx) {

}

void TCompMovableObject::registerMsgs() {
    DECL_MSG(TCompMovableObject, TMsgObjectBeingMoved, onObjectBeingMoved);
}

void TCompMovableObject::update(float dt) {
    if (isBeingMoved) {
        CEntity* eMovingMe = hEntityMovingMe;
        TCompTransform * myPos = get<TCompTransform>();
        TCompTransform* eMovingMeTransform = eMovingMe->get<TCompTransform>();
        VEC3 myPosition = myPos->getPosition();
        myPos->setPosition(myPosition + direction * speed * dt);
    }
}
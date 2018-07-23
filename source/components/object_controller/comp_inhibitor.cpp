#include "mcv_platform.h"
#include "comp_inhibitor.h"
#include "components/comp_transform.h"
#include "render/render_objects.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("inhibitor", TCompInhibitor);

void TCompInhibitor::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
    CEntity* parent = h_parent;
    TCompTransform* c_parent_trans = parent->get<TCompTransform>();

    TCompTransform *c_my_trans = get<TCompTransform>();
    c_my_trans->setPosition(c_parent_trans->getPosition() + VEC3(0, 2, 0));

    float parent_yaw, parent_pitch;
    c_parent_trans->getYawPitchRoll(&parent_yaw, &parent_pitch);

    float my_yaw, my_pitch;
    c_my_trans->getYawPitchRoll(&my_yaw, &my_pitch);
    c_my_trans->setYawPitchRoll(parent_yaw, my_pitch);
}

void TCompInhibitor::onMsgLaunchInhibitor(const TMsgLaunchInhibitor & msg)
{
    TCompTransform* myPos = get<TCompTransform>();

    /* Calcular el tiempo que le va a costar llegar */
    CEntity* player = getEntityByName("The Player");
    TCompTransform* playerPos = player->get<TCompTransform>();
    dest = playerPos->getPosition();
    float distance = VEC3::Distance2D(playerPos->getPosition(), myPos->getPosition());

    time = mapInRange(time_range.x, time_range.y, 0, msg.maxDistance + 0.5f, distance);

    float y = playerPos->getPosition().y - myPos->getPosition().y;
    time = sqrtf(2 * y / -9.8f);
    speed = distance / time;
    //time = 
    /* Lerpear posición para que moviendo con esa rotación se quede clavado en el suelo */
}

void TCompInhibitor::debugInMenu() {

}

void TCompInhibitor::load(const json& j, TEntityParseContext& ctx) {
    min_x_speed = j.value("min_speed", 2.f);
    time_range = loadVEC2(j["time_range"]);
    h_parent = ctx.entity_starting_the_parse;
}

void TCompInhibitor::registerMsgs() {
    DECL_MSG(TCompInhibitor, TMsgEntityCreated, onMsgEntityCreated);
    DECL_MSG(TCompInhibitor, TMsgLaunchInhibitor, onMsgLaunchInhibitor);
}

void TCompInhibitor::update(float dt) {
    currentTime += dt;
    TCompTransform* myPos = get<TCompTransform>();
    if (VEC3::Distance(myPos->getPosition(), dest) > 0.1f) {
        VEC3 newPos = VEC3::Zero;
        //newPos.y = 
    }
}
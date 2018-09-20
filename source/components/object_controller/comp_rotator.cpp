#include "mcv_platform.h"
#include "comp_rotator.h"
#include "components/comp_transform.h"
#include "components/comp_hierarchy.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("rotator", TCompRotator);

void TCompRotator::debugInMenu() {
    ImGui::DragFloat("Speed", &_speed, 0.1f, 0.f, 10.f);
}

void TCompRotator::load(const json& j, TEntityParseContext& ctx) {

    _speed = j.value("speed", .65f);
}

void TCompRotator::registerMsgs()
{

}

void TCompRotator::update(float dt) {

    TCompTransform * self_transform = get<TCompTransform>();
    float yaw, pitch, roll;
    self_transform->getYawPitchRoll(&yaw, &pitch, &roll);
    self_transform->setYawPitchRoll(yaw + (dt * _speed), pitch, roll);
}
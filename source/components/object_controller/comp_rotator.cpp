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
    ImGui::DragFloat("Speed", &_speed, 0.01f, 0.f, 10.f);
}

void TCompRotator::load(const json& j, TEntityParseContext& ctx) {

    _speed = j.value("speed", .65f);
    _acceleration = j.value("acceleration", 0.f);
    _max_acceleration = j.value("max_acceleration", 0.f);
    _rotate_pitch = j.value("rotate_pitch", false);
	_rotate_roll = j.value("rotate_roll", false);
}

void TCompRotator::registerMsgs()
{

}

void TCompRotator::update(float dt) {

    if (!CHandle(this).getOwner().isValid())
        return;

    TCompTransform * self_transform = get<TCompTransform>();
    float yaw, pitch, roll;

    _total_accel += _total_accel > _max_acceleration ? 0 : _acceleration * dt;
    self_transform->getYawPitchRoll(&yaw, &pitch, &roll);
    //dbg("total pitch %f\n", pitch);
	if (_rotate_roll)
		self_transform->setYawPitchRoll(yaw, pitch, roll + (dt * _speed) + _total_accel);
    else if(_rotate_pitch)
        self_transform->setYawPitchRoll(yaw, pitch + (dt * _speed) + _total_accel, roll);
    else
        self_transform->setYawPitchRoll(yaw + (dt * _speed) + _total_accel, pitch, roll);
}
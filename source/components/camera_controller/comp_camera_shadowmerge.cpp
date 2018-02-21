#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_camera_shadowmerge.h"
#include "../comp_transform.h"

DECL_OBJ_MANAGER("camera_shadowmerge", TCompCameraShadowMerge);

// Same as the thirdperson camera by now
// This will need to be changed for vertical shadow merging.

void TCompCameraShadowMerge::debugInMenu()
{
	ImGui::DragFloat3("Offsets", &_clipping_offset.x, 0.1f, -20.f, 20.f);
	ImGui::DragFloat2("Angles", &_clamp_angle.x, 0.1f, -90.f, 90.f);
	ImGui::DragFloat("Speed", &_speed, 0.1f, 0.f, 20.f);
}

void TCompCameraShadowMerge::load(const json& j, TEntityParseContext& ctx)
{
	// Read from the json all the input data
	_speed = j.value("speed", 1.5f);
	_target_name = j.value("target", "");
	_clamp_angle = loadVEC2(j["clampangle"]);
	_clipping_offset = loadVEC3(j["offset"]);
	_clamp_angle = VEC2(deg2rad(_clamp_angle.x), deg2rad(_clamp_angle.y));

	// Load the target and set his axis as our axis.
	_h_target = ctx.findEntityByName(_target_name);
	TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>();

	float yaw, pitch, roll;
	target_transform->getYawPitchRoll(&yaw, &pitch, &roll);
	_current_euler = VEC2(yaw, pitch);

}

void TCompCameraShadowMerge::update(float dt)
{
	if (!_h_target.isValid())
		return;

	TCompTransform* self_transform = get<TCompTransform>();
	TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>(); // we will need to consume this.
	assert(self_transform);
	assert(target_transform);

	// To remove in the future.
	float horizontal_delta = EngineInput.mouse()._position_delta.x;
	float vertical_delta = -EngineInput.mouse()._position_delta.y;
	if (btRHorizontal.isPressed()) horizontal_delta = btRHorizontal.value;
	if (btRVertical.isPressed()) vertical_delta = btRVertical.value;

	// Verbose code
	_current_euler.x -= horizontal_delta * _speed * dt;
	_current_euler.y += vertical_delta * _speed * dt;
	_current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, -_clamp_angle.x);

	// EulerAngles method based on mcv class
	VEC3 vertical_offset = VEC3::Up * _clipping_offset.y; // Change VEC3::up, for the players vertical angle, (TARGET VERTICAL)
	VEC3 horizontal_offset = self_transform->getLeft() * _clipping_offset.x;
	VEC3 target_position = target_transform->getPosition() + vertical_offset + horizontal_offset;

	self_transform->setPosition(target_position);
	self_transform->setYawPitchRoll(_current_euler.x, _current_euler.y, 0);
	VEC3 new_pos = target_position + _clipping_offset.z * -self_transform->getFront();
	self_transform->setPosition(new_pos);

	float inputSpeed = Clamp(fabs(btHorizontal.value) + fabs(btVertical.value), 0.f, 1.f);
	float current_fov = 70 + inputSpeed * 30; // Just doing some testing with the fov and speed
	setPerspective(deg2rad(current_fov), 0.1f, 1000.f);
}

VEC3 TCompCameraShadowMerge::CameraClipping(void)
{
	return VEC3();
}

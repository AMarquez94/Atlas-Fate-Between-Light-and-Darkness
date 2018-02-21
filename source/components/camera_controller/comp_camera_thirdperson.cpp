#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_camera_thirdperson.h"
#include "../comp_transform.h"

DECL_OBJ_MANAGER("camera_thirdperson", TCompCameraThirdPerson);
const Input::TInterface_Mouse& mouse = EngineInput.mouse();

void TCompCameraThirdPerson::debugInMenu()
{
	ImGui::DragFloat3("Offsets", &_clipping_offset.x, 0.1f, -20.f, 20.f);
	ImGui::DragFloat2("Angles", &_clamp_angle.x, 0.1f, -90.f, 90.f);
	ImGui::DragFloat("Speed", &_speed, 0.1f, 0.f, 20.f);
}

void TCompCameraThirdPerson::load(const json& j, TEntityParseContext& ctx)
{
	// Read from the json all the input data
	_speed = j.value("speed", 1.0f);
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

void TCompCameraThirdPerson::update(float dt)
{
	if (!_h_target.isValid())
		return;

	TCompTransform* self_transform = get<TCompTransform>();
	TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>(); // we will need to consume this.
	assert(self_transform);
	assert(target_transform);

	// To remove in the future.
	float horizontal_delta = -mouse._position_delta.x;
	float vertical_delta = mouse._position_delta.y;
	if (btHorizontal.isPressed()) horizontal_delta = btHorizontal.value;
	if (btVertical.isPressed()) vertical_delta = btVertical.value;

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
}

VEC3 TCompCameraThirdPerson::CameraClipping(void)
{
	//TO-DO
	// Raycast from player direction to desired camera pos
	// Return the position

	return VEC3();
}

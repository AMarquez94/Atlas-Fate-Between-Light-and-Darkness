#include "mcv_platform.h"
#include "comp_camera_orbit.h"
#include "components/comp_transform.h"
#include "utils/utils.h"

DECL_OBJ_MANAGER("camera_orbit", TCompCameraOrbit);

void TCompCameraOrbit::debugInMenu() {
	ImGui::DragFloat("Sensitivity", &_sensitivity, 0.01f, 0.001f, 0.1f);
}

void TCompCameraOrbit::load(const json& j, TEntityParseContext& ctx) {
	_sensitivity = j.value("sensitivity", _sensitivity);
}

void TCompCameraOrbit::update(float dt)
{
	TCompTransform* c_transform = get<TCompTransform>();
	VEC3 pos = c_transform->getPosition();
	VEC3 front = c_transform->getFront();

	// target
	VEC3 targetPos = VEC3::Zero;
	if (_target.isValid())
	{
		CEntity* e_target = _target;
		TCompTransform* c_tgt_transform = e_target->get<TCompTransform>();
		targetPos = c_tgt_transform->getPosition();
	}

	// rotation
	float yaw, pitch;
	getYawPitchFromVector(front, &yaw, &pitch);

	auto& mouse = EngineInput.mouse();
	if (mouse.button(Input::MOUSE_RIGHT).isPressed())
	{
		VEC2 mOff = mouse._position_delta;
		yaw += -mOff.x * _sensitivity;
		pitch += -mOff.y * _sensitivity;
		pitch = Clamp(pitch, -_maxPitch, _maxPitch);
	}

	// final values
	VEC3 newFront = getVectorFromYawPitch(yaw, pitch);
	VEC3 newPos = targetPos - newFront * _distance;

	c_transform->lookAt(newPos, targetPos);
}

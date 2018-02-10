#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_flyover_controller.h"
#include "comp_transform.h"

DECL_OBJ_MANAGER("flyover_controller", TCompFlyOverController);

void TCompFlyOverController::debugInMenu() {
	ImGui::DragFloat("Rotation Sensibility", &rotation_sensibility, 0.001f, 0.001f, 0.01f);
	ImGui::DragFloat("Pan Sensibility", &pan_sensibility, 0.01f, 0.1f, 5.f);
	ImGui::DragFloat("Inertia", &speed_reduction_factor, 0.001f, 0.7f, 1.f);
}

void TCompFlyOverController::load(const json& j, TEntityParseContext& ctx) {
	rotation_sensibility = j.value("rotation_sensibility", rotation_sensibility);
	pan_sensibility = j.value("pan_sensibility", pan_sensibility);
	speed_reduction_factor = j.value("speed_reduction_factor", speed_reduction_factor);
}

void TCompFlyOverController::update(float dt) {

	TCompTransform* c_transform = get<TCompTransform>();

	VEC3 origin = c_transform->getPosition();

	speed *= speed_reduction_factor;

	float dyaw = 0.f, dpitch = 0.f;

	VEC3 front = c_transform->getFront();
	float yaw, pitch;
	getYawPitchFromVector(front, &yaw, &pitch);

	VEC2 prev_cursor = cursor;
	cursor = VEC2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	VEC2 delta_cursor = cursor - prev_cursor;

	if (ImGui::IsMouseDown(1)) {
		if (delta_cursor.x || delta_cursor.y) {
			dyaw -= delta_cursor.x;
			dpitch -= delta_cursor.y;
		}

		if (isPressed('W'))
			speed.z += 1.f;
		if (isPressed('S'))
			speed.z -= 1.f;
		if (isPressed('A'))
			speed.x += 1.f;
		if (isPressed('D'))
			speed.x -= 1.f;
	}

	yaw = yaw + dyaw * rotation_sensibility;
	pitch = pitch + dpitch * rotation_sensibility;
	const float max_pitch = deg2rad(90.f - 1e-2f);
	if (pitch > max_pitch)
		pitch = max_pitch;
	else if (pitch < -max_pitch)
		pitch = -max_pitch;

	origin += c_transform->getLeft() * pan_sensibility * speed.x * dt;
	origin += c_transform->getFront() * pan_sensibility * speed.z * dt;
	origin += c_transform->getUp() * pan_sensibility * speed.y * dt;

	VEC3 new_front = getVectorFromYawPitch(yaw, pitch);
	VEC3 target = origin + new_front;
	c_transform->lookAt(origin, target);
}








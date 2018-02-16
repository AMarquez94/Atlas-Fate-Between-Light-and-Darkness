#include "mcv_platform.h"
#include "comp_camera.h"
#include "comp_transform.h"

DECL_OBJ_MANAGER("camera", TCompCamera);

void TCompCamera::debugInMenu() {
	float fov_deg = rad2deg(getFov());
	float new_znear = getZNear();
	float new_zfar = getZFar();
	bool changed = ImGui::DragFloat("Fov", &fov_deg, 0.1f, 30.f, 175.f);
	changed |= ImGui::DragFloat("Z Near", &new_znear, 0.001f, 0.01f, 1.0f);
	changed |= ImGui::DragFloat("Z Far", &new_zfar, 1.0f, 1.0f, 3000.0f);
	if (changed)
		setPerspective(deg2rad(fov_deg), new_znear, new_zfar);
}

void TCompCamera::load(const json& j, TEntityParseContext& ctx) {

  // ..
	float fov_deg = j.value("fov", rad2deg(getFov()));
	float z_near = j.value("z_near", getZNear());
	float z_far = j.value("z_far", getZFar());
  setPerspective(deg2rad(fov_deg), z_near, z_far);

}

void TCompCamera::update(float dt) {

  TCompTransform* c = get<TCompTransform>();
  assert(c);
  this->lookAt(c->getPosition(), c->getPosition() + c->getFront(), c->getUp());

}


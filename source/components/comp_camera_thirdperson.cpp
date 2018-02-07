#include "mcv_platform.h"
#include "comp_camera.h"
#include "comp_transform.h"

DECL_OBJ_MANAGER("camera", TCompCamera);

void TCompCamera::debugInMenu() {

}

void TCompCamera::load(const json& j, TEntityParseContext& ctx) {

  // ..
  float fov_deg = j.value("fov", 75.f);
  float z_near = j.value("z_near", 0.1f);
  float z_far = j.value("z_far", 1000.f);
  setPerspective(deg2rad(fov_deg), z_near, z_far);

}

void TCompCamera::update(float dt) {

  TCompTransform* c = get<TCompTransform>();
  assert(c);
  this->lookAt(c->getPosition(), c->getPosition() + c->getFront(), c->getUp());

}


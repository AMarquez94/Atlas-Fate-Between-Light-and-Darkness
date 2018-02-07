#pragma once

#include "comp_base.h"
#include "camera/camera.h"

class TCompCamera : public CCamera, public TCompBase {
public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  DECL_SIBLING_ACCESS();
};
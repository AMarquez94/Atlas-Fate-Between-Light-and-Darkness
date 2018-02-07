#pragma once

#include "comp_base.h"
#include "geometry/transform.h"

class TCompPlayerController : public TCompBase {
  float   speedFactor = 1.0f;
  float   rotationSpeed = 1.0f;
  VEC3    speed;

  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
};
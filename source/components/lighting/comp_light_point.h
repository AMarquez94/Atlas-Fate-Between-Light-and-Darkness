#pragma once

#include "../comp_base.h"

class TCompLightPoint : public TCompBase {

  // Light params
  VEC4            color = VEC4(1, 1, 1, 1);
  float           intensity = 1.0f;
  float           radius = 1.0f;

public:
  void debugInMenu();
  void renderDebug();
  void load(const json& j, TEntityParseContext& ctx);
  DECL_SIBLING_ACCESS();

  void activate();
  MAT44 getWorld();
};
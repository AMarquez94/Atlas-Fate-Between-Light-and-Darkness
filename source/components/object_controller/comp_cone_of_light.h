#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompConeOfLightController : public TCompBase {
  
  CEntity* player;
  float fov;
  float dist;
  bool turnedOn;
  std::string defaultMesh;

  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void turnOnLight();
};
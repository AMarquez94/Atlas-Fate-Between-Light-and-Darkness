#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompConeOfLightController : public TCompBase {
  
  CEntity* player;
  float fov;
  float dist;
  bool turnedOn;

  DECL_SIBLING_ACCESS();

  void onMsgEntityCreated(const TMsgEntityCreated& msg);

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  static void registerMsgs();

  void turnOnLight();
  void turnOffLight();

};
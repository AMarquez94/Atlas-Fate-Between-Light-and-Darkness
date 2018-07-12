#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompConeOfLightController : public TCompBase {
  
  CHandle h_player;
  float hor_fov;
  float ver_fov;
  float dist;
  bool turnedOn;

  VEC3 origin = VEC3::Zero;
  VEC3 dest = VEC3::Zero;

  DECL_SIBLING_ACCESS();

  void onMsgGroupCreated(const TMsgEntitiesGroupCreated& msg);

public:
  bool playerIlluminated = false;

  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  static void registerMsgs();

  void turnOnLight();
  void turnOffLight();

  bool isPlayerHiddenFromLight();

};
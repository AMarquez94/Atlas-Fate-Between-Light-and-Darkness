#pragma once

#include "comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

struct TCompBulletController : public TCompBase {
  
  CHandle h_sender;
  float collision_radius = 0.25f;
  float speed = 1.0f;

  void onAssignBulletOwner(const TMsgAssignBulletOwner& msg);

  DECL_SIBLING_ACCESS();

public:
  static void registerMsgs();
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
};
#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompMovableObject : public TCompBase {
  
  DECL_SIBLING_ACCESS();

  bool isBeingMoved = false;
  CHandle hEntityMovingMe = CHandle();
  VEC3 direction;
  float speed;
  void onObjectBeingMoved(const TMsgObjectBeingMoved& msg);

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  static void registerMsgs();
};
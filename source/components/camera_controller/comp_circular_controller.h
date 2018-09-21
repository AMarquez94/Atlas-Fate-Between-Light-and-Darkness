#pragma once

#include "../comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompCircularController : public TCompBase {

  VEC2        radius;
  CHandle     h_target;
  std::string target_name;

  // Use engine's one
  float       total_time = 0.f;

  float       speed = 1.0f;
  float       vertical_speed = 0.f;
  float       oscilation_speed = 0.f;

  float       curr_yaw = 0.f;
  float       curr_height = 0.f;
  float       oscilation_range = 0.f;

  void onCreate(const TMsgEntityCreated& msg);
  void onNewTarget(const TMsgCircularControllerTarget& msg);

  DECL_SIBLING_ACCESS();

public:
  static void registerMsgs();
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
};
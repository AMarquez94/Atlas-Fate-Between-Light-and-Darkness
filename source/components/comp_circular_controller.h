#pragma once

#include "comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompCircularController : public TCompBase {

  float       radius = 1.0f;
  CHandle     h_target;
  std::string target_name;
  float       speed = 1.0f;
  float       curr_yaw = 0.f;

  void onCreate(const TMsgEntityCreated& msg);

  DECL_SIBLING_ACCESS();

public:
  static void registerMsgs();
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
};
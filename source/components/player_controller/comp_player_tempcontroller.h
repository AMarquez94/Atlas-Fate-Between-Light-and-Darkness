#pragma once

#include "components/comp_base.h"

struct TMsgAction {
  std::string action_name;
  DECL_MSG_ID();
};

class TCompTempPlayerController : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  static void registerMsgs();
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

private:
  void onAction(const TMsgAction& msg);

  std::string _action_name;
  float _time = 0.f;
};


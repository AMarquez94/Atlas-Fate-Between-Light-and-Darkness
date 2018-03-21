#pragma once

#include "components/comp_base.h"

struct TMsgAction {
  std::string action_name;
  DECL_MSG_ID();
};

typedef void (TCompTempPlayerController::*statehandler)(float);

class TCompTempPlayerController : public TCompBase
{
  DECL_SIBLING_ACCESS();

  std::string                         stateName;
  statehandler						  state;
  // the states, as maps to functions
  std::map<std::string, statehandler> statemap;

public:
  static void registerMsgs();
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  void playerMotion(float dt);

private:
  void onAction(const TMsgAction& msg);

  std::string _action_name;
  float _time = 0.f;
};


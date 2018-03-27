#pragma once

#include "components/comp_base.h"

class CCurve;

class TCompTriggerCamera : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  static void registerMsgs();

private:

  CHandle _targetCamera;
  std::string _targetName;

  void onMsgTriggerEnter(const TMsgTriggerEnter& msg);
  void onMsgTriggerExit(const TMsgTriggerExit& msg);
};


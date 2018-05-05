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
	float time = 0.0f;
	bool onCamera = false;
  CHandle _targetCamera;
  std::string _targetName;

  float _timeToExitCamera;
  float _blendInTime;
  float _blendOutTime;
  std::string _interpolator; //TODO

  void onMsgTriggerEnter(const TMsgTriggerEnter& msg);
  void onMsgTriggerExit(const TMsgTriggerExit& msg);
};


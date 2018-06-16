#pragma once

#include "components/comp_base.h"

class TCompNoiseEmitter : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  void makeNoise(float noiseRadius, float timeToRepeat, bool isNoise, bool isOnlyOnce, bool isArtificial);

  void changeNoiseRadius(float newRadius);

  static void registerMsgs();

private:

  float _timeToRepeatNoise;
  float _timer = 0.f;
  bool _once = false;
  bool _onceNoiseMade = false;
  bool _isNoise = false;
  bool _artificial = false;
  physx::PxSphereGeometry emitter_sphere;
  physx::PxQueryFilterData PxNoiseEmitterFilterData;

  void onMsgScenePaused(const TMsgScenePaused & msg);

};


#pragma once

#include "comp_base.h"
#include "geometry/transform.h"
#include "ia/ai_controller.h"
#include "entity/common_msgs.h"

class TCompPlayerController : public IAIController {

	/* Attributes */
  float   speedFactor = 1.0f;
  float   rotationSpeed = 1.0f;
  VEC3    speed;

  /* Timers */

  /* Keys */
  const Input::TInterface_Keyboard& kb = CEngine::get().getInput().host(Input::PLAYER_1).keyboard();

  DECL_SIBLING_ACCESS();

  void onMsgDamage(const TMsgDamage& msg);

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void Init();
  
  static void registerMsgs();

  /* States */
  void IdleState(float);
  void MotionState(float);	//Movement
  void PushState(float);
  void AttackState(float);
  void ProbeState(float);
  void RemovingInhibitorState(float);
  void ShadowMergingEnterState(float);
  void ShadowMergingHorizontalState(float);
  void ShadowMergingVerticalState(float);
  void ShadowMergingEnemyState(float);
  void ShadowMergingLandingState(float);
  void ShadowMergingExitState(float);
  void FallingState(float);
  void LandingState(float);
  void HitState(float);
  void DeadState(float);
};
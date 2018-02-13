#pragma once

#include "comp_base.h"
#include "geometry/transform.h"
#include "ia/ai_controller.h"
#include "entity/common_msgs.h"

class TCompPlayerController : public IAIController {

	/* Attributes */
	std::map<std::string, CRenderMesh*> mesh_states;

	float stamina;
	float maxStamina;
	float minStamina;
	float minStaminaToMerge;
	float dcrStaminaOnPlaceMultiplier;
	float dcrStaminaGround;
	float dcrStaminaWall;
	float incrStamina = 20.f;

	float walkSpeedFactor;
	float walkCrouchSpeedFactor;
	float runSpeedFactor;
	float walkSlowSpeedFactor;
	float walkSlowCrouchSpeedFactor;
	float currentSpeed;
	float rotationSpeed;
	//These two parameters describe the behaviour of analogs.
	//RotationNeeded changes the amount of horizontal tilt the players need to apply when the player is going straight forward
	//in order to start going in diagonal instead of forward.

	/* Timers */

	/* Keys */
	const Input::TButton& btUp = EngineInput["btUp"];
	const Input::TButton& btDown = EngineInput["btDown"];
	const Input::TButton& btRight = EngineInput["btRight"];
	const Input::TButton& btLeft = EngineInput["btLeft"];
	const Input::TButton& btShadowMerging = EngineInput["btShadowMerging"];
	const Input::TButton& btAttack = EngineInput["btAttack"];
	const Input::TButton& btRun = EngineInput["btRun"];
	const Input::TButton& btSlow = EngineInput["btSlow"];
	const Input::TButton& btAction = EngineInput["btAction"];
	const Input::TButton& btCrouch = EngineInput["btCrouch"];


	const Input::TButton& btHorizontal = EngineInput["Horizontal"];
	const Input::TButton& btVertical = EngineInput["Vertical"];

	/* TODO: not for milestone 1 */
	//const Input::TButton& btCrouch = EngineInput["w"];	
	//const Input::TButton& btSecAction = EngineInput["w"];
	std::string target_name;

	DECL_SIBLING_ACCESS();

	void onMsgDamage(const TMsgDamage& msg);

	/* Private aux functions */
	bool motionButtonsPressed();
	void movePlayer(float);

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void Init();
  
	static void registerMsgs();

	/* States */
	void IdleState(float);
	void MotionState(float);	//Movement
	void CrouchState(float);
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

	bool checkShadows();
};
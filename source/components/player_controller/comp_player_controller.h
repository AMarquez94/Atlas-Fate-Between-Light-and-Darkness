#pragma once

#include "../comp_base.h"
#include "geometry/transform.h"
#include "../ia/ai_controller.h"
#include "entity/common_msgs.h"

class TCompPlayerController : public IAIController {

	std::string auxStateName = "";
	std::map<std::string, CRenderMesh*> mesh_states;

	/* Camera stack, to bypass entity delayed loading */
	/* Replace everything here with a real camera stack */
	std::string camera_shadowmerge;
	std::string camera_thirdperson;
	std::string camera_actual;

	/* Player stamina */
	float stamina;
	float maxStamina;
	float minStamina;
	float minStaminaToMerge;
	float dcrStaminaOnPlaceMultiplier;
	float dcrStaminaGround;
	float dcrStaminaWall;
	float incrStamina = 20.f;
	
	/* Player speeds*/
	float walkSpeedFactor;
	float walkSlowSpeedFactor;
	float walkCrouchSpeedFactor;
	float walkSlowCrouchSpeedFactor;
	float currentSpeed;
	float rotationSpeed;
	float runSpeedFactor;

	/* Aux offset */
	float maxGroundDistance = 0.3f;

	/* Timers */
	int timesToPressRemoveInhibitorKey;
	int timesRemoveInhibitorKeyPressed = 0;
	float timerForPressingRemoveInhibitorKey = 0.f;

	/* Private aux functions */
	void movePlayer(float);
	void movePlayerShadow(float);
	void manageInhibition(float dt);

	const bool ConcaveTest(void);
	const bool ConvexTest(void);
	const bool ShadowTest(void);
	const bool GroundTest(void);
	const bool motionButtonsPressed(void);

	DECL_SIBLING_ACCESS();

	/* Messages handled by the player */
	void onMsgDamage(const TMsgDamage& msg);
	void onMsgPlayerHit(const TMsgPlayerHit& msg);
	void onMsgPlayerShotInhibitor(const TMsgInhibitorShot& msg);

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
	const Input::TButton& btSecAction = EngineInput["btSecAction"];
	const Input::TButton& btHorizontal = EngineInput["Horizontal"];
	const Input::TButton& btVertical = EngineInput["Vertical"];

public:

	bool isGrounded;
	bool inhibited = false;
	VEC3 delta_movement;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void Init();
  
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

	static void registerMsgs();

};
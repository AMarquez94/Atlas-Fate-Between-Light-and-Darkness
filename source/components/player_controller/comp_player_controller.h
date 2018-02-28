#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "components/ia/ai_controller.h"
#include "entity/common_msgs.h"

class TCompPlayerController : public IAIController {

	std::map<std::string, CRenderMesh*> mesh_states;

	/* Camera stack, to bypass entity delayed loading */
	/* Replace everything here with a real camera stack */
	std::string camera_shadowmerge_hor;
	std::string camera_shadowmerge_ver;
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

	float dcrStaminaGroundAux = 0.f;
	float dcrStaminaWallAux = 0.f;
	
	/* Player speeds*/
	float runSpeedFactor;
	float walkSpeedFactor;
	float walkSlowSpeedFactor;
	float walkCrouchSpeedFactor;
	float walkSlowCrouchSpeedFactor;
	float currentSpeed;
	float rotationSpeed;
	float distToAttack;
	float distToSM;
	float canAttack = false;
	CHandle enemyToAttack;
	CHandle enemyToSM;

	bool crouched = false;

	/* Aux offset */
	float maxGroundDistance = 0.3f;
	float convexMaxDistance = 0.55f;

	/* Timers */
	int timesToPressRemoveInhibitorKey;
	int timesRemoveInhibitorKeyPressed = 0;
	float timerForPressingRemoveInhibitorKey = 0.f;

	DECL_SIBLING_ACCESS();

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
	const Input::TButton& btDebugShadows = EngineInput["btDebugShadows"];

	/* TODO: not for milestone 1 */
	//const Input::TButton& btCrouch = EngineInput["w"];	
	//const Input::TButton& btSecAction = EngineInput["w"];
	std::string target_name;
	bool inhibited = false;

	void onMsgPlayerHit(const TMsgPlayerHit& msg);
	void onMsgPlayerShotInhibitor(const TMsgInhibitorShot& msg);
	void onMsgPlayerIlluminated(const TMsgPlayerIlluminated& msg);


	/* Aux variables */
	std::string auxStateName = "";
	
	/* Private aux functions */

	void movePlayer(const float dt);
	void movePlayerShadow(const float dt);
	bool manageInhibition(float dt);
	void ResetPlayer(void);
	const bool motionButtonsPressed();
	void allowAttack(bool allow, CHandle enemy);
	CHandle checkTouchingStunnedEnemy();
	bool checkEnemyInShadows(CHandle enemy);
	void manageCrouch();
	bool playerInFloor();
	bool canStandUp();


	const bool ConcaveTest(void);
	const bool ConvexTest(void);
	const bool ShadowTest(void);
	const bool GroundTest(void);

public:

	//VEC3 delta_movement;

	void debugInMenu();
	void renderDebug();
	void load(const json& j, TEntityParseContext& ctx);
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

	const bool checkPaused();

	//bool checkShadows();
	const bool isInShadows();
	const bool isDead();
	const bool checkAttack();
};
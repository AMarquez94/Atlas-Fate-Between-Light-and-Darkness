#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"
#include "components/player_controller/comp_player_animator.h"
#include "gui/gui_widget.h"

class TCompTempPlayerController;

typedef void (TCompTempPlayerController::*actionfinish)();
typedef void (TCompTempPlayerController::*actionhandler)(float);

struct TargetCamera {

	std::string name;
	float blendIn;
	float blendOut;
};

struct Noise {
	bool isOnlyOnce;
	float noiseRadius;
	float timeToRepeat;
	bool isNoise;
	bool isArtificial;
};

struct TMsgStateStart {
	
	actionhandler action_start;
	float speed;
	float size;
	float radius;

	TargetCamera * target_camera = nullptr;

	Noise * noise = nullptr;

	DECL_MSG_ID();
};

struct TMsgStateFinish {

	actionfinish action_finish;
	std::string meshname;
	float speed;

	DECL_MSG_ID();
};

class TCompTempPlayerController : public TCompBase
{
	struct StateColors {
		VEC4 colorIdle;
		VEC4 colorMerge;
		VEC4 colorDead;
		VEC4 colorInhib;
	}playerColor;

	//Debug console message variables
	bool infinite, immortal, hackShadows, invisible;
	float speedBoost = 1.0;

	/* DEPRECATED */
	std::map<std::string, CRenderMesh*> mesh_states;

	physx::PxFilterData * pxPlayerFilterData;
	physx::PxFilterData * pxShadowFilterData;
	physx::PxQueryFilterData PxPlayerDiscardQuery;

	actionhandler state;
	CHandle target_camera;

	bool tempInverseVerticalMovementMerged = false;
	float *staminaBarValue;

	float mergeAngle;
	float currentSpeed;
	float rotationSpeed;
	float fallingTime;
	float maxFallingTime;
	float hardFallingTime;
	float fallingDistance;
	float maxFallingDistance;
	float maxAttackDistance;
	std::string auxCamera;

	float stamina;
	float minStamina;
	float maxStamina;
	float incrStamina;
	float decrStaticStamina;
	float decrStaminaHorizontal;
	float decrStaminaVertical;
	float minStaminaChange;
	
	int timesRemoveInhibitorKeyPressed = 0;
	float timeToPressAgain = 0.7f;
	float timeInhib = 0.0f;

    float attackTimer = 0.f;    //HARD FIX: TODO: Remove

	void onCreate(const TMsgEntityCreated& msg);
	void onStateStart(const TMsgStateStart& msg);
	void onStateFinish(const TMsgStateFinish& msg);

	void onPlayerHit(const TMsgPlayerHit& msg);
	void onPlayerKilled(const TMsgPlayerDead& msg);
	void onPlayerInhibited(const TMsgInhibitorShot& msg);
	void onPlayerExposed(const TMsgPlayerIlluminated& msg);
	void onPlayerPaused(const TMsgScenePaused& msg);
	
	//Console hacks
	void onConsoleChanged(const TMsgConsoleOn& msg);
	void onInfiniteStamina(const TMsgInfiniteStamina& msg);
	void onPlayerImmortal(const TMsgPlayerImmortal& msg);
	void onPlayerMove(const TMsgPlayerMove& msg);
	void onPlayerInShadows(const TMsgPlayerInShadows& msg);
	void onSpeedBoost(const TMsgSpeedBoost& msg);
	void onPlayerInvisible(const TMsgPlayerInvisible& msg);

	void onShadowChange(const TMsgShadowChange& msg);

	DECL_SIBLING_ACCESS();

public:
	/* Debug variables */
	bool dbgDisableStamina;
	std::string dbCameraState;
	float stepTimer = 0.0f; 
	bool stepRight = true;
  //VEC3 debugDir = VEC3::Zero;
  //VEC3 debugMyFront = VEC3::Zero;

	bool isMerged;
	bool isGrounded;
	bool isInhibited;
	unsigned int initialPoints;
	unsigned int hitPoints;

	void debugInMenu();
	void renderDebug();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	/* State functions */
	void walkState(float dt);
	void idleState(float dt);
	void deadState(float dt);
	void mergeState(float dt);
	void attackState(float dt);
	void resetState(float dt);
	void exitMergeState(float dt);
	void removingInhibitorState(float dt);

	/* Player condition tests */
	const bool concaveTest(void);
	const bool convexTest(void);
	const bool onMergeTest(float dt);
	const bool groundTest(float dt);

	/* Auxiliar functions */
	void updateStamina(float dt);
	void updateShader(float dt);
	void mergeEnemy();
	void resetMerge();
	bool isDead();
	void upButtonReselased();

	VEC3 getMotionDir(const VEC3 & front, const VEC3 & left);
	CHandle closeEnemy(const std::string & state = "undefined");
	CHandle closestEnemyToStun();

	static void registerMsgs();
};


#pragma once

#include "components/comp_base.h"
#include "comp_bt_enemy.h"
#include "modules/module_ia.h"

class TCompAIMimetic : public TCompAIEnemy {

private:

    /* Atributes */

    enum EType { FLOOR = 0, WALL, NUM_TYPES };

    float speed = 3.f;
    float rotationSpeedChaseDeg = 359.f;
    float rotationSpeedChase;
    std::string entityToChase = "The Player";
    float fovDeg = 120.f;
    float fov;
    float autoChaseDistance = 10.f;
    float maxChaseDistance = 35.f;
    float maxTimeSuspecting = 3.f;
    float dcrSuspectO_Meter = .3f;
    float incrBaseSuspectO_Meter = .5f;
    float distToAttack = 1.5f;

	/* Completed Animations Checkers */

	bool wakeUpJumpAnimationCompleted = false;
	bool wakeUpAnimationCompleted = false;

	bool restJumpAnimationCompleted = false;
	bool restAnimationCompleted = false;


    /* Timers */
    float timerWaitingInObservation = 0.f;
    float timerToExplode = 0.f;

    bool isSlept = false;
    EType type;
    bool isActive = false;
    bool goingInactive = false;
    VEC3 initialPos;
    VEC3 initialLookAt;
    float rotationSpeedObservation = deg2rad(50.f);
    float rotationSpeedPatrolling = deg2rad(80.f);
    float waitTimeInLasPlayerPos = 3.f;
    float chaseSpeed = 6.f;

    float amountRotatedObserving = 0.f;
    float maxAmountRotateObserving = deg2rad(45.f);

    float rotationSpeedNoise = deg2rad(120.f);

	CHandle laser_handle;

    VEC3 pushedDirection = VEC3::Zero;
    float pushedTime = 0.f;

    DECL_SIBLING_ACCESS();

    void onMsgEntityCreated(const TMsgEntityCreated& msg);
    void onMsgPlayerDead(const TMsgPlayerDead& msg);
    void onMsgMimeticStunned(const TMsgEnemyStunned& msg);
    void onMsgNoiseListened(const TMsgNoiseMade& msg);
    void onMsgPhysxContact(const TMsgPhysxContact& msg);
	void onMsgAnimationCompleted(const TMsgAnimationCompleted& msg);

    /* Aux functions */
    void setGravityToFaceWall();
    EType parseStringMimeticType(const std::string& typeString);
	bool isPlayerOnLaser();

    //load
    void loadActions() override;
    void loadConditions() override;
    void loadAsserts() override;

public:
    void preUpdate(float dt) override;
    void postUpdate(float dt) override;
    void load(const json& j, TEntityParseContext& ctx) override;
    void debugInMenu();

    /* ACTIONS */
    BTNode::ERes actionStunned(float dt);
    BTNode::ERes actionExplode(float dt);
    BTNode::ERes actionResetObserveVariables(float dt);
	BTNode::ERes actionWaitOnWall(float dt);
    BTNode::ERes actionObserveLeft(float dt);
    BTNode::ERes actionObserveRight(float dt);
    BTNode::ERes actionWaitObserving(float dt);
    BTNode::ERes actionSetActive(float dt);
	BTNode::ERes actionSetActiveOnWall(float dt);
    BTNode::ERes actionGenerateNavmeshWpt(float dt);
    BTNode::ERes actionGoToWpt(float dt);
    BTNode::ERes actionResetTimerWaiting(float dt);
    BTNode::ERes actionWaitInWpt(float dt);
    BTNode::ERes actionNextWpt(float dt);
    BTNode::ERes actionSleep(float dt);
    BTNode::ERes actionWakeUp(float dt);
    BTNode::ERes actionJumpFloor(float dt);
    BTNode::ERes actionResetVariablesChase(float dt);
    BTNode::ERes actionRotateTowardsPlayerWithNoise(float dt);
    BTNode::ERes actionChasePlayerWithNoise(float dt);
    BTNode::ERes actionMarkNoiseAsInactive(float dt);
    BTNode::ERes actionGenerateNavmeshNoiseSource(float dt);
    BTNode::ERes actionGoToNoiseSource(float dt);
    BTNode::ERes actionWaitInNoiseSource(float dt);
    BTNode::ERes actionSuspect(float dt);
    BTNode::ERes actionRotateToNoiseSource(float dt);
    BTNode::ERes actionGenerateNavmeshPlayerLastPos(float dt);
    BTNode::ERes actionGoToPlayerLastPos(float dt);
    BTNode::ERes actionWaitInPlayerLastPos(float dt);
    BTNode::ERes actionSetGoInactive(float dt);
    BTNode::ERes actionGenerateNavmeshInitialPos(float dt);
    BTNode::ERes actionGoToInitialPos(float dt);
    BTNode::ERes actionRotateToInitialPos(float dt);
    BTNode::ERes actionJumpWall(float dt);
    BTNode::ERes actionHoldOnWall(float dt);
    BTNode::ERes actionSetInactive(float dt);
    BTNode::ERes actionClosestWpt(float dt);

    /* CONDITIONS */
    bool conditionHasBeenStunned(float dt);
    bool conditionIsTypeWall(float dt);
    bool conditionIsNotPlayerInFovAndNotNoise(float dt);
	bool conditionIsNotPlayerInRaycastAndNotNoise(float dt);
    bool conditionIsNotActive(float dt);
    bool conditionHasWpts(float dt);
    bool conditionIsTypeFloor(float dt);
    bool conditionIsSlept(float dt);
    bool conditionHasNotWaypoints(float dt);
    bool conditionNotListenedNoise(float dt);
    bool conditionPlayerSeenForSure(float dt);
    bool conditionHasHeardArtificialNoise(float dt);
    bool conditionNotSurePlayerInFov(float dt);
    bool conditionHasHeardNaturalNoise(float dt);
    bool conditionIsPlayerInFov(float dt);
    bool conditionNotGoingInactive(float dt);
    bool conditionIsDestUnreachable(float dt);

    /* ASSERTS */
    bool assertNotPlayerInFovNorNoise(float dt);
	bool assertNotPlayerInRaycastNorNoise(float dt);
    bool assertNotPlayerInFov(float dt);
    bool assertNotPlayerInFovNorArtificialNoise(float dt);
    bool assertCantReachDest(float dt);
    bool assertCanReachDest(float dt);

    const std::string getStateForCheckpoint();

    static void registerMsgs();

    void playAnimationByName(const std::string & animationName) override;

	void registerLaserHandle(CHandle h_laser);

	void setLaserState(bool state);

	void resetAnimationCompletedBooleans();

};
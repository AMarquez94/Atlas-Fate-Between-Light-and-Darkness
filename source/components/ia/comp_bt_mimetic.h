#pragma once

#include "components/comp_base.h"
#include "comp_bt.h"
#include "modules/module_ia.h"

class TCompAIMimetic : public TCompIAController {

private:

	/* Atributes */
	std::vector<Waypoint> _waypoints;
	int currentWaypoint;

	enum EType { FLOOR = 0, WALL, NUM_TYPES };

	float speed = 3.f;
	float rotationSpeedChaseDeg = 359.f;
	float rotationSpeedChase;
	std::string entityToChase = "The Player";
	float fovDeg = 179.f;
	float fov;
	float autoChaseDistance = 15.f;
	float maxChaseDistance = 35.f;
	float maxTimeSuspecting = 3.f;
	float suspectO_Meter = 0.f;
	float dcrSuspectO_Meter = .3f;
	float incrBaseSuspectO_Meter = .5f;
	float distToAttack = 1.5f;
	VEC3 lastPlayerKnownPos = VEC3::Zero;
	bool startLightsOn = false;
	bool alarmEnded = true;
	bool hasBeenStunned = false;
	bool isLastPlayerKnownDirLeft = false;
	bool hasHeardArtificialNoise = false;
	bool hasHeardNaturalNoise = false;
	VEC3 noiseSource = VEC3::Zero;

	std::string validState = "";

	/* Timers */
	float timerWaitingInWpt = 0.f;
	float timerWaitingInObservation = 0.f;
	float timerToExplode = 0.f;

	bool isSlept = false;
	EType type;
	bool isActive = false;
	bool goingInactive = false;
	VEC3 initialPos;
	VEC3 initialLookAt;
	float rotationSpeedObservation = deg2rad(40.f);
	float waitTimeInLasPlayerPos = 3.f;
	float chaseSpeed = 6.f;

	float amountRotatedObserving = 0.f;
	float maxAmountRotateObserving = deg2rad(45.f);

	DECL_SIBLING_ACCESS();

	void onMsgEntityCreated(const TMsgEntityCreated& msg);
	void onMsgPlayerDead(const TMsgPlayerDead& msg);
	void onMsgMimeticStunned(const TMsgEnemyStunned& msg);
	void onMsgNoiseListened(const TMsgNoiseMade& msg);

	/* Aux functions */
	const Waypoint getWaypoint() { return _waypoints[currentWaypoint]; }
	void addWaypoint(const Waypoint& wpt) { _waypoints.push_back(wpt); };
	bool rotateTowardsVec(VEC3 objective, float rotationSpeed, float dt);
	bool isPlayerInFov();
	bool isEntityHidden(CHandle hEntity);
	void turnOnLight();
	void turnOffLight();
	void setGravityToFaceWall();
	EType parseStringMimeticType(const std::string& typeString);
	
	//load
	void loadActions() override;
	void loadConditions() override;
	void loadAsserts() override;

public:
	void load(const json& j, TEntityParseContext& ctx) override;
	void debugInMenu();

	/* ACTIONS */
	BTNode::ERes actionStunned(float dt);
	BTNode::ERes actionExplode(float dt);
	BTNode::ERes actionResetObserveVariables(float dt);
	BTNode::ERes actionObserveLeft(float dt);
	BTNode::ERes actionObserveRight(float dt);
	BTNode::ERes actionWaitObserving(float dt);
	BTNode::ERes actionSetActive(float dt);
	BTNode::ERes actionGoToWpt(float dt);
	BTNode::ERes actionResetTimerWaiting(float dt);
	BTNode::ERes actionWaitInWpt(float dt);
	BTNode::ERes actionNextWpt(float dt);
	BTNode::ERes actionSleep(float dt);
	BTNode::ERes actionWakeUp(float dt);
	BTNode::ERes actionJumpFloor(float dt);
	BTNode::ERes actionResetVariablesChase(float dt);
	BTNode::ERes actionChasePlayerWithNoise(float dt);
	BTNode::ERes actionMarkNoiseAsInactive(float dt);
	BTNode::ERes actionGoToNoiseSource(float dt);
	BTNode::ERes actionWaitInNoiseSource(float dt);
	BTNode::ERes actionSuspect(float dt);
	BTNode::ERes actionRotateToNoiseSource(float dt);
	BTNode::ERes actionGoToPlayerLastPos(float dt);
	BTNode::ERes actionWaitInPlayerLastPos(float dt);
	BTNode::ERes actionSetGoInactive(float dt);
	BTNode::ERes actionGoToInitialPos(float dt);
	BTNode::ERes actionRotateToInitialPos(float dt);
	BTNode::ERes actionJumpWall(float dt);
	BTNode::ERes actionHoldOnWall(float dt);
	BTNode::ERes actionSetInactive(float dt);

	/* CONDITIONS */
	bool conditionHasBeenStunned(float dt);
	bool conditionIsTypeWall(float dt);
	bool conditionIsNotPlayerInFovAndNotNoise(float dt);
	bool conditionIsNotActive(float dt);
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

	/* ASSERTS */
	bool assertNotPlayerInFovNorNoise(float dt);
	bool assertNotPlayerInFov(float dt);

	bool isStunned() { return current && current->getName().compare("stunned") == 0; }

	static void registerMsgs();
};
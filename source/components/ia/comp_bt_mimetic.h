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

	float speed = 3.5f;
	float rotationSpeedDeg = 90.0f;
	float rotationSpeed;
	std::string entityToChase = "The Player";
	float fovDeg = 70.f;
	float fov;
	float autoChaseDistance = 15.f;
	float maxChaseDistance = 35.f;
	float maxTimeSuspecting = 3.f;
	float suspectO_Meter = 0.f;
	float dcrSuspectO_Meter = .3f;
	float incrBaseSuspectO_Meter = .5f;
	bool isLastPlayerKnownDirLeft = false;
	float distToAttack = 1.5f;
	float amountRotated = 0.f;
	float maxRotationSeekingPlayerDeg = 90.f;
	float maxRotationSeekingPlayer;
	VEC3 lastPlayerKnownPos = VEC3::Zero;
	VEC3 lastStunnedPatrolKnownPos = VEC3::Zero;
	bool startLightsOn = false;
	bool alarmEnded = true;
	bool hasBeenStunned = false;
	bool hasBeenShadowMerged = false;
	bool hasBeenFixed = false;

	std::string validState = "";

	/* Timers */
	float timerWaitingInWpt = 0.f;

	bool isSlept;
	EType type;
	bool isActive;
	bool goingInactive;

	DECL_SIBLING_ACCESS();

	void onMsgEntityCreated(const TMsgEntityCreated& msg);
	void onMsgPlayerDead(const TMsgPlayerDead& msg);
	void onMsgMimeticStunned(const TMsgEnemyStunned& msg);

	/* Aux functions */
	const Waypoint getWaypoint() { return _waypoints[currentWaypoint]; }
	void addWaypoint(const Waypoint& wpt) { _waypoints.push_back(wpt); };
	void rotateTowardsVec(VEC3 objective, float dt);
	bool isPlayerInFov();
	bool isEntityHidden(CHandle hEntity);
	void turnOnLight();
	void turnOffLight();
	
	//load
	void loadActions() override;
	void loadConditions() override;
	void loadAsserts() override;

public:
	void load(const json& j, TEntityParseContext& ctx) override;
	void debugInMenu();

	/* ACTIONS */
	BTNode::ERes actionStunned(float dt);
	BTNode::ERes actionObserve(float dt);
	BTNode::ERes actionSetActive(float dt);
	BTNode::ERes actionJumpFloor(float dt);
	BTNode::ERes actionGoToWpt(float dt);
	BTNode::ERes actionWaitInWpt(float dt);
	BTNode::ERes actionNextWpt(float dt);
	BTNode::ERes actionSleep(float dt);
	BTNode::ERes actionWakeUp(float dt);
	BTNode::ERes actionSuspect(float dt);
	BTNode::ERes actionChasePlayerWithNoise(float dt);
	BTNode::ERes actionGoToPlayerLastPos(float dt);
	BTNode::ERes actionWaitInPlayerLastPos(float dt);
	BTNode::ERes actionSetGoInactive(float dt);
	BTNode::ERes actionGoToInitialPos(float dt);
	BTNode::ERes actionJumpWall(float dt);
	BTNode::ERes actionSetInactive(float dt);

	/* CONDITIONS */
	bool conditionHasBeenStunned(float dt);
	bool conditionIsTypeWall(float dt);
	bool conditionIsNotPlayerInFov(float dt);
	bool conditionIsNotActive(float dt);
	bool conditionIsTypeFloor(float dt);
	bool conditionIsSlept(float dt);
	bool conditionHasNotWaypoints(float dt);
	bool conditionNotListenedNoise(float dt);
	bool conditionNotSurePlayerInFov(float dt);
	bool conditionPlayerSeenForSure(float dt);
	bool conditionIsPlayerInFov(float dt);
	bool conditionNotGoingInactive(float dt);

	/* ASSERTS */
	bool assertNotPlayerInFov(float dt);



	static void registerMsgs();
};
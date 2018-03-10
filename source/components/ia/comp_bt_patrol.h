#pragma once

#include "components/comp_base.h"
#include "comp_bt.h"

struct Waypoint {
	VEC3 position;
	VEC3 lookAt;				//TODO: guess how to make the waypoint orientated to something
	float minTime;
};

class TCompAIPatrol : public TCompIAController {

private:

	/* Atributes */
	std::vector<Waypoint> _waypoints;
	int currentWaypoint;

	float speed = 5.5f;
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

	std::string validState = "";

	/* Timers */
	float timerWaitingInWpt = 0.f;

	DECL_SIBLING_ACCESS();

	void onMsgEntityCreated(const TMsgEntityCreated& msg);

	/* Aux functions */
	const Waypoint getWaypoint() { return _waypoints[currentWaypoint]; }
	void addWaypoint(const Waypoint& wpt) { _waypoints.push_back(wpt); };
	void rotateTowardsVec(VEC3 objective, float dt);
	bool isPlayerInFov();
	bool isEntityHidden(CHandle hEntity);
	void turnOnLight();
	void turnOffLight();

public:
	void load(const json& j, TEntityParseContext& ctx) override;
	void debugInMenu();

	BTNode::ERes actionShadowMerged(float dt);
	BTNode::ERes actionStunned(float dt);
	BTNode::ERes actionFixed(float dt);
	BTNode::ERes actionBeginAlert(float dt);
	BTNode::ERes actionClosestWpt(float dt);
	BTNode::ERes actionEndAlert(float dt);
	BTNode::ERes actionGoToWpt(float dt);
	BTNode::ERes actionWaitInWpt(float dt);
	BTNode::ERes actionNextWpt(float dt);
	BTNode::ERes actionSuspect(float dt);
	BTNode::ERes actionShootInhibitor(float dt);
	BTNode::ERes actionChasePlayer(float dt);
	BTNode::ERes actionAttack(float dt);
	BTNode::ERes actionGoToPlayerLastPos(float dt);
	BTNode::ERes actionLookForPlayer(float dt);
	BTNode::ERes actionMarkPatrolAsSeen(float dt);
	BTNode::ERes actionGoToPatrol(float dt);
	BTNode::ERes actionFixPatrol(float dt);
	BTNode::ERes actionGoToPatrolPrevPos(float dt);

	bool conditionManageStun(float dt);
	bool conditionEndAlert(float dt);
	bool conditionShadowMerged(float dt);
	bool conditionFixed(float dt);
	bool conditionPlayerSeen(float dt);
	bool conditionPlayerWasSeen(float dt);
	bool conditionPatrolSeen(float dt);
	bool conditionPatrolWasSeen(float dt);
	bool conditionGoToWpt(float dt);
	bool conditionWaitInWpt(float dt);
	bool conditionChase(float dt);
	bool conditionPlayerAttacked(float dt);

	bool assertGoToWpt(float dt);
	bool assertWaitInWpt(float dt);
	bool assertPlayerInFov(float dt);
	bool assertPlayerNotInFov(float dt);
	bool assertPlayerNotInFovAndPatrolInFov(float dt);
	bool assertPlayerAndPatrolNotInFov(float dt);

	static void registerMsgs();
};
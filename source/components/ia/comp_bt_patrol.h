#pragma once

#include "components/comp_base.h"
#include "comp_bt.h"
#include "modules/module_ia.h"

class TCompAIPatrol : public TCompIAController {

private:

  struct StateColors {
    VEC4 colorNormal;
    VEC4 colorSuspect;
    VEC4 colorAlert;
    VEC4 colorDead;
  } patrolColor;

	/* Atributes */
	std::vector<Waypoint> _waypoints;
	int currentWaypoint;

	float suspectO_Meter = 0.f;
	bool isLastPlayerKnownDirLeft = false;
	float amountRotated = 0.f;
	VEC3 lastPlayerKnownPos = VEC3::Zero;
	VEC3 lastStunnedPatrolKnownPos = VEC3::Zero;
	bool startLightsOn = false;
	bool alarmEnded = true;
	bool hasBeenStunned = false;
	bool hasBeenShadowMerged = false;
	bool hasBeenFixed = false;

  /* TODO: create struct for noises */
	bool hasHeardNaturalNoise = false;
	bool hasHeardArtificialNoise = false;
	VEC3 noiseSource = VEC3::Zero;
  bool noiseSourceChanged = false;
  CHandle hNoiseSource = CHandle();
  std::chrono::steady_clock::time_point lastTimeNoiseWasHeard;

  float maxDistanceToNavmeshPoint = 3.f;

	std::string validState = "";

  std::vector<VEC3> navmeshPath;
  int navmeshPathPoint = 0;
  bool recalculateNavmesh = false;

	/* Timers */
	float timerWaitingInWpt = 0.f;
	float timerWaitingInNoise = 0.f;

	DECL_SIBLING_ACCESS();

	void onMsgEntityCreated(const TMsgEntityCreated& msg);
	void onMsgPlayerDead(const TMsgPlayerDead& msg);
	void onMsgPatrolStunned(const TMsgEnemyStunned& msg);
	void onMsgPatrolShadowMerged(const TMsgPatrolShadowMerged& msg);
	void onMsgPatrolFixed(const TMsgPatrolFixed& msg);
	void onMsgNoiseListened(const TMsgNoiseMade& msg);

	/* Aux functions */
	const Waypoint getWaypoint() { return _waypoints[currentWaypoint]; }
	void addWaypoint(const Waypoint& wpt) { _waypoints.push_back(wpt); };
	bool rotateTowardsVec(VEC3 objective, float dt, float rotationSpeed);
	bool isPlayerInFov(std::string entityToChase, float fov, float maxChaseDistance);
	bool isEntityHidden(CHandle hEntity);
	void turnOnLight();
	void turnOffLight();
	bool isStunnedPatrolInFov(float fov, float maxChaseDistance);
	bool isStunnedPatrolInPos(VEC3 lastPos);
	CHandle getPatrolInPos(VEC3 lastPos);
  void generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc = false);
  bool moveToPoint(float speed, float rotationSpeed, VEC3 destPoint, float dt);
	
	//load
	void loadActions() override;
	void loadConditions() override;
	void loadAsserts() override;

public:
	void load(const json& j, TEntityParseContext& ctx) override;
	void debugInMenu();

	BTNode::ERes actionShadowMerged(float dt);
	BTNode::ERes actionStunned(float dt);
	BTNode::ERes actionFixed(float dt);
	BTNode::ERes actionBeginAlert(float dt);
	BTNode::ERes actionClosestWpt(float dt);
	BTNode::ERes actionEndAlert(float dt);
	BTNode::ERes actionMarkNoiseAsInactive(float dt);
  BTNode::ERes actionGenerateNavmeshNoise(float dt);
	BTNode::ERes actionGoToNoiseSource(float dt);
	BTNode::ERes actionWaitInNoiseSource(float dt);
  BTNode::ERes actionGenerateNavmeshWpt(float dt);
	BTNode::ERes actionGoToWpt(float dt);
	BTNode::ERes actionWaitInWpt(float dt);
	BTNode::ERes actionNextWpt(float dt);
	BTNode::ERes actionSuspect(float dt);
	BTNode::ERes actionMarkPlayerAsSeen(float dt);
	BTNode::ERes actionShootInhibitor(float dt);
  BTNode::ERes actionGenerateNavmeshChase(float dt);
	BTNode::ERes actionChasePlayer(float dt);
	BTNode::ERes actionAttack(float dt);
	BTNode::ERes actionRotateToNoiseSource(float dt);
	BTNode::ERes actionResetPlayerWasSeenVariables(float dt);
	BTNode::ERes actionGoToPlayerLastPos(float dt);
	BTNode::ERes actionLookForPlayer(float dt);
  BTNode::ERes actionGenerateNavmeshGoToPatrol(float dt);
	BTNode::ERes actionGoToPatrol(float dt);
	BTNode::ERes actionFixPatrol(float dt);
	BTNode::ERes actionMarkPatrolAsLost(float dt);

	bool conditionManageStun(float dt);
	bool conditionEndAlert(float dt);
	bool conditionShadowMerged(float dt);
	bool conditionFixed(float dt);
	bool conditionIsArtificialNoise(float dt);
	bool conditionIsNaturalNoise(float dt);
	bool conditionPlayerSeen(float dt);
	bool conditionPlayerWasSeen(float dt);
	bool conditionPatrolSeen(float dt);
	bool conditionFixPatrol(float dt);
	bool conditionGoToWpt(float dt);
	bool conditionWaitInWpt(float dt);
	bool conditionChase(float dt);
	bool conditionPlayerAttacked(float dt);

	bool assertPlayerInFov(float dt);
	bool assertPlayerNotInFov(float dt);
	bool assertPlayerAndPatrolNotInFov(float dt);
	bool assertNotHeardArtificialNoise(float dt);
	bool assertNotPlayerInFovNorArtificialNoise(float dt);
	bool assertPlayerNotInFovNorNoise(float dt);
	bool assertPlayerAndPatrolNotInFovNotNoise(float dt);

	bool isStunned() { return current && current->getName().compare("stunned") == 0; }

	static void registerMsgs();
};
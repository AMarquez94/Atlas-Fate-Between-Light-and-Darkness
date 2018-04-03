#include "mcv_platform.h"
#include "comp_bt_patrol.h"
#include "btnode.h"
#include "components/comp_name.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_controller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/object_controller/comp_cone_of_light.h"
#include "components/physics/comp_rigidbody.h"
#include "components/physics/comp_collider.h"
#include "physics/physics_collider.h"


DECL_OBJ_MANAGER("ai_patrol", TCompAIPatrol);

void TCompAIPatrol::debugInMenu() {

	TCompIAController::debugInMenu();

	if (current) {
		validState = current->getName();
	}

	ImGui::Text("Current state: %s", validState.c_str());

	ImGui::Text("Suspect Level:");
	ImGui::SameLine();
	ImGui::ProgressBar(suspectO_Meter);

	ImGui::Text("Last player pos: %f, %f, %f", lastPlayerKnownPos.x, lastPlayerKnownPos.y, lastPlayerKnownPos.z);
}

void TCompAIPatrol::load(const json& j, TEntityParseContext& ctx) {

	loadActions();
	loadConditions();
	loadAsserts();
	TCompIAController::loadTree(j);
	TCompIAController::loadParameters(j);

	if (j.count("waypoints") > 0) {
		auto& j_waypoints = j["waypoints"];
		_waypoints.clear();
		for (auto it = j_waypoints.begin(); it != j_waypoints.end(); ++it) {

			Waypoint wpt;
			assert(it.value().count("position") == 1);
			assert(it.value().count("lookAt") == 1);

			wpt.position = loadVEC3(it.value()["position"]);
			wpt.lookAt = loadVEC3(it.value()["lookAt"]);
			wpt.minTime = it.value().value("minTime", 5.f);
			addWaypoint(wpt);
		}
	}
	startLightsOn = j.value("startLightsOn", false);
	currentWaypoint = 0;
	/* TODO: ¿Init node? */
}

void TCompAIPatrol::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
	TCompName *tName = get<TCompName>();
	name = tName->getName();

	if (_waypoints.size() == 0) {

		TCompTransform * tPos = get<TCompTransform>();
		Waypoint wpt;
		wpt.position =tPos->getPosition();
		wpt.lookAt = tPos->getFront();
		wpt.minTime = 1.f;
		addWaypoint(wpt);
	}

	if (startLightsOn) {
		turnOnLight();
	}
}

void TCompAIPatrol::onMsgPlayerDead(const TMsgPlayerDead& msg) {

	alarmEnded = false;
	if (!isPatrolStunned()) {

		/* We reset the timer if we are not stunned*/
		current = nullptr;
	}
}

void TCompAIPatrol::onMsgPatrolStunned(const TMsgEnemyStunned & msg)
{
	hasBeenStunned = true;

	TCompRender *cRender = get<TCompRender>();
	cRender->color = VEC4(1, 1, 1, 1);
	TCompTransform *mypos = get<TCompTransform>();
	float y, p, r;
	mypos->getYawPitchRoll(&y, &p, &r);
	p = p + deg2rad(89.f);
	mypos->setYawPitchRoll(y, p, r);
	turnOffLight();

	TCompGroup* cGroup = get<TCompGroup>();
	CEntity* eCone = cGroup->getHandleByName("Cone of Vision");
	TCompRender * coneRender = eCone->get<TCompRender>();
	coneRender->visible = false;

	lastPlayerKnownPos = VEC3::Zero;

	/* Tell the other patrols I am stunned */
	CEngine::get().getIA().patrolSB.stunnedPatrols.emplace_back(CHandle(this).getOwner());

	current = nullptr;
}

void TCompAIPatrol::onMsgPatrolShadowMerged(const TMsgPatrolShadowMerged & msg)
{
	hasBeenShadowMerged = true;

	TCompRender *cRender = get<TCompRender>();
	cRender->color = VEC4(0, 0, 0, 0);

	/* Stop telling the other patrols that I am stunned */
	bool found = false;
	std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;
	for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
		if (stunnedPatrols[i] == CHandle(this).getOwner()) {
			found = true;
			stunnedPatrols.erase(stunnedPatrols.begin() + i);
		}
	}

	current = nullptr;
}

void TCompAIPatrol::onMsgPatrolFixed(const TMsgPatrolFixed & msg)
{
	if (isPatrolStunned()) {

		hasBeenFixed = true;

		TCompTransform *mypos = get<TCompTransform>();
		float y, p, r;
		mypos->getYawPitchRoll(&y, &p, &r);
		p = p - deg2rad(89.f);
		mypos->setYawPitchRoll(y, p, r);
		turnOnLight();

		TCompGroup* cGroup = get<TCompGroup>();
		CEntity* eCone = cGroup->getHandleByName("Cone of Vision");
		TCompRender * coneRender = eCone->get<TCompRender>();
		coneRender->visible = true;

		/* Stop telling the other patrols that I am stunned */
		bool found = false;
		std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;
		for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
			if (stunnedPatrols[i] == CHandle(this).getOwner()) {
				found = true;
				stunnedPatrols.erase(stunnedPatrols.begin() + i);
			}
		}

		current = nullptr;
	}
}

void TCompAIPatrol::registerMsgs()
{
	DECL_MSG(TCompAIPatrol, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompAIPatrol, TMsgPlayerDead, onMsgPlayerDead);
	DECL_MSG(TCompAIPatrol, TMsgEnemyStunned, onMsgPatrolStunned);
	DECL_MSG(TCompAIPatrol, TMsgPatrolShadowMerged, onMsgPatrolShadowMerged);
	DECL_MSG(TCompAIPatrol, TMsgPatrolFixed, onMsgPatrolFixed);
}

void TCompAIPatrol::loadActions() {
	actions_initializer.clear();

	actions_initializer["actionShadowMerged"] = (BTAction)&TCompAIPatrol::actionShadowMerged;
	actions_initializer["actionStunned"] = (BTAction)&TCompAIPatrol::actionStunned;
	actions_initializer["actionFixed"] = (BTAction)&TCompAIPatrol::actionFixed;
	actions_initializer["actionBeginAlert"] = (BTAction)&TCompAIPatrol::actionBeginAlert;
	actions_initializer["actionClosestWpt"] = (BTAction)&TCompAIPatrol::actionClosestWpt;
	actions_initializer["actionEndAlert"] = (BTAction)&TCompAIPatrol::actionEndAlert;
	actions_initializer["actionGoToWpt"] = (BTAction)&TCompAIPatrol::actionGoToWpt;
	actions_initializer["actionWaitInWpt"] = (BTAction)&TCompAIPatrol::actionWaitInWpt;
	actions_initializer["actionNextWpt"] = (BTAction)&TCompAIPatrol::actionNextWpt;
	actions_initializer["actionSuspect"] = (BTAction)&TCompAIPatrol::actionSuspect;
	actions_initializer["actionMarkPlayerAsSeen"] = (BTAction)&TCompAIPatrol::actionMarkPlayerAsSeen;
	actions_initializer["actionShootInhibitor"] = (BTAction)&TCompAIPatrol::actionShootInhibitor;
	actions_initializer["actionChasePlayer"] = (BTAction)&TCompAIPatrol::actionChasePlayer;
	actions_initializer["actionAttack"] = (BTAction)&TCompAIPatrol::actionAttack;
	actions_initializer["actionResetPlayerWasSeenVariables"] = (BTAction)&TCompAIPatrol::actionResetPlayerWasSeenVariables;
	actions_initializer["actionGoToPlayerLastPos"] = (BTAction)&TCompAIPatrol::actionGoToPlayerLastPos;
	actions_initializer["actionLookForPlayer"] = (BTAction)&TCompAIPatrol::actionLookForPlayer;
	actions_initializer["actionGoToPatrol"] = (BTAction)&TCompAIPatrol::actionGoToPatrol;
	actions_initializer["actionFixPatrol"] = (BTAction)&TCompAIPatrol::actionFixPatrol;
	actions_initializer["actionMarkPatrolAsLost"] = (BTAction)&TCompAIPatrol::actionMarkPatrolAsLost;

}

void TCompAIPatrol::loadConditions() {
	conditions_initializer.clear();

	conditions_initializer["conditionManageStun"] = (BTCondition)&TCompAIPatrol::conditionManageStun;
	conditions_initializer["conditionEndAlert"] = (BTCondition)&TCompAIPatrol::conditionEndAlert;
	conditions_initializer["conditionShadowMerged"] = (BTCondition)&TCompAIPatrol::conditionShadowMerged;
	conditions_initializer["conditionFixed"] = (BTCondition)&TCompAIPatrol::conditionFixed;
	conditions_initializer["conditionPlayerSeen"] = (BTCondition)&TCompAIPatrol::conditionPlayerSeen;
	conditions_initializer["conditionPlayerWasSeen"] = (BTCondition)&TCompAIPatrol::conditionPlayerWasSeen;
	conditions_initializer["conditionPatrolSeen"] = (BTCondition)&TCompAIPatrol::conditionPatrolSeen;
	conditions_initializer["conditionFixPatrol"] = (BTCondition)&TCompAIPatrol::conditionFixPatrol;
	conditions_initializer["conditionGoToWpt"] = (BTCondition)&TCompAIPatrol::conditionGoToWpt;
	conditions_initializer["conditionWaitInWpt"] = (BTCondition)&TCompAIPatrol::conditionWaitInWpt;
	conditions_initializer["conditionChase"] = (BTCondition)&TCompAIPatrol::conditionChase;
	conditions_initializer["conditionPlayerAttacked"] = (BTCondition)&TCompAIPatrol::conditionPlayerAttacked;

}

void TCompAIPatrol::loadAsserts() {
	asserts_initializer.clear();

	asserts_initializer["assertPlayerInFov"] = (BTCondition)&TCompAIPatrol::assertPlayerInFov;
	asserts_initializer["assertPlayerNotInFov"] = (BTCondition)&TCompAIPatrol::assertPlayerNotInFov;
	asserts_initializer["assertPlayerAndPatrolNotInFov"] = (BTCondition)&TCompAIPatrol::assertPlayerAndPatrolNotInFov;

}

/* ACTIONS */

BTNode::ERes TCompAIPatrol::actionShadowMerged(float dt)
{
	/* Destroy the entity */
	CHandle(this).getOwner().destroy();
	return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPatrol::actionStunned(float dt)
{
	return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPatrol::actionFixed(float dt)
{
	hasBeenStunned = false;
	hasBeenFixed = false;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionBeginAlert(float dt)
{
	turnOnLight();
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionClosestWpt(float dt)
{
	float minDistance = INFINITY;
	int  minIndexWpt = 0;

	TCompTransform *mypos = get<TCompTransform>();

	for (int i = 0; i < _waypoints.size(); i++) {
		float currDistance = VEC3::Distance(mypos->getPosition(), _waypoints[i].position);
		if (currDistance < minDistance) {
			minDistance = currDistance;
			minIndexWpt = i;
		}
	}

	currentWaypoint = minIndexWpt;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionEndAlert(float dt)
{
	turnOffLight();
	suspectO_Meter = 0.f;
	TCompRender *cRender = get<TCompRender>();
	cRender->color = VEC4(1, 1, 1, 1);
	lastPlayerKnownPos = VEC3::Zero;
	alarmEnded = true;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionGoToWpt(float dt)
{
	assert(arguments.find("speed_actionGoToWpt_goToWpt") != arguments.end());
	float speed = arguments["speed_actionGoToWpt_goToWpt"].getFloat();
	assert(arguments.find("rotationSpeed_actionGoToWpt_goToWpt") != arguments.end());
	float rotationSpeed = deg2rad(arguments["rotationSpeed_actionGoToWpt_goToWpt"].getFloat());

	TCompTransform *mypos = get<TCompTransform>();
	rotateTowardsVec(getWaypoint().position, dt, rotationSpeed);

	VEC3 vp = mypos->getPosition();
	if (VEC3::Distance(getWaypoint().position, vp) < speed * dt) {
		mypos->setPosition(getWaypoint().position);
		return BTNode::ERes::LEAVE;
	}
	else {
		VEC3 vfwd = mypos->getFront();
		vfwd.Normalize();
		vp = vp + speed * dt *vfwd;
		mypos->setPosition(vp);				//Move towards wpt
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIPatrol::actionWaitInWpt(float dt)
{
	assert(arguments.find("rotationSpeed_actionWaitInWpt_waitInWpt") != arguments.end());
	float rotationSpeed = deg2rad(arguments["rotationSpeed_actionWaitInWpt_waitInWpt"].getFloat());
	
	if (timerWaitingInWpt >= getWaypoint().minTime) {
		return BTNode::ERes::LEAVE;
	}
	else {
		timerWaitingInWpt += dt;
		TCompTransform *mypos = get<TCompTransform>();
		rotateTowardsVec(mypos->getPosition() + getWaypoint().lookAt, dt, rotationSpeed);
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIPatrol::actionNextWpt(float dt)
{
	timerWaitingInWpt = 0.f;
	currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionSuspect(float dt)
{
	assert(arguments.find("rotationSpeed_actionSuspect_suspect") != arguments.end());
	float rotationSpeed = deg2rad(arguments["rotationSpeed_actionSuspect_suspect"].getFloat());
	assert(arguments.find("fov_actionSuspect_suspect") != arguments.end());
	float fov = deg2rad(arguments["fov_actionSuspect_suspect"].getFloat());
	assert(arguments.find("autoChaseDistance_actionSuspect_suspect") != arguments.end());
	float autoChaseDistance = arguments["autoChaseDistance_actionSuspect_suspect"].getFloat();
	assert(arguments.find("maxChaseDistance_actionSuspect_suspect") != arguments.end());
	float maxChaseDistance = arguments["maxChaseDistance_actionSuspect_suspect"].getFloat();
	assert(arguments.find("dcrSuspectO_Meter_actionSuspect_suspect") != arguments.end());
	float dcrSuspectO_Meter = arguments["dcrSuspectO_Meter_actionSuspect_suspect"].getFloat();
	assert(arguments.find("incrBaseSuspectO_Meter_actionSuspect_suspect") != arguments.end());
	float incrBaseSuspectO_Meter = arguments["incrBaseSuspectO_Meter_actionSuspect_suspect"].getFloat();
	assert(arguments.find("entityToChase_actionSuspect_suspect") != arguments.end());
	std::string entityToChase = arguments["entityToChase_actionSuspect_suspect"].getString();
	
	TCompRender *cRender = get<TCompRender>();
	cRender->color = VEC4(255, 255, 0, 1);
	// chase
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	/* Distance to player */
	float distanceToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());

	if (distanceToPlayer <= autoChaseDistance && isPlayerInFov(entityToChase, fov, maxChaseDistance)) {
		suspectO_Meter = 1.f;
		rotateTowardsVec(ppos->getPosition(), dt, rotationSpeed);
	}
	else if (distanceToPlayer <= maxChaseDistance && isPlayerInFov(entityToChase, fov, maxChaseDistance)) {
		suspectO_Meter = Clamp(suspectO_Meter + dt * incrBaseSuspectO_Meter, 0.f, 1.f);							//TODO: increment more depending distance and noise
		rotateTowardsVec(ppos->getPosition(), dt, rotationSpeed);
	}
	else {
		suspectO_Meter = Clamp(suspectO_Meter - dt * dcrSuspectO_Meter, 0.f, 1.f);
	}

	if (suspectO_Meter <= 0.f || suspectO_Meter >= 1.f) {
		if (suspectO_Meter <= 0) {
			cRender->color = VEC4(1, 1, 1, 1);
		}
		return BTNode::ERes::LEAVE;
	}
	else {
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIPatrol::actionMarkPlayerAsSeen(float dt)
{
	assert(arguments.find("entityToChase_actionMarkPlayerAsSeen_markPlayerAsSeen") != arguments.end());
	std::string entityToChase = arguments["entityToChase_actionMarkPlayerAsSeen_markPlayerAsSeen"].getString();
	
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform * ppos = player->get<TCompTransform>();
	lastPlayerKnownPos = ppos->getPosition();
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionShootInhibitor(float dt)
{
	assert(arguments.find("entityToChase_actionShootInhibitor_shootInhibitor") != arguments.end());
	std::string entityToChase = arguments["entityToChase_actionShootInhibitor_shootInhibitor"].getString();
	
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompPlayerController *pController = player->get<TCompPlayerController>();
	TCompRender *cRender = get<TCompRender>();

	cRender->color = VEC4(255, 0, 0, 1);

	if (!pController->isInhibited()) {
		TMsgInhibitorShot msg;
		msg.h_sender = CHandle(this).getOwner();
		player->sendMsg(msg);
	}
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionChasePlayer(float dt)
{
	assert(arguments.find("speed_actionChasePlayer_ChasePlayer") != arguments.end());
	float speed = arguments["speed_actionChasePlayer_ChasePlayer"].getFloat();
	assert(arguments.find("rotationSpeed_actionChasePlayer_ChasePlayer") != arguments.end());
	float rotationSpeed = deg2rad(arguments["rotationSpeed_actionChasePlayer_ChasePlayer"].getFloat());
	assert(arguments.find("fov_actionChasePlayer_ChasePlayer") != arguments.end());
	float fov = deg2rad(arguments["fov_actionChasePlayer_ChasePlayer"].getFloat());
	assert(arguments.find("maxChaseDistance_actionChasePlayer_ChasePlayer") != arguments.end());
	float maxChaseDistance = arguments["maxChaseDistance_actionChasePlayer_ChasePlayer"].getFloat();
	assert(arguments.find("distToAttack_actionChasePlayer_ChasePlayer") != arguments.end());
	float distToAttack = arguments["distToAttack_actionChasePlayer_ChasePlayer"].getFloat();
	assert(arguments.find("entityToChase_actionChasePlayer_ChasePlayer") != arguments.end());
	std::string entityToChase = arguments["entityToChase_actionChasePlayer_ChasePlayer"].getString();

	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	isStunnedPatrolInFov(fov, maxChaseDistance);

	if (lastPlayerKnownPos != VEC3::Zero) {

		/* If we had the player's previous position, know where he is going */
		isLastPlayerKnownDirLeft = mypos->isInLeft(ppos->getPosition() - lastPlayerKnownPos);
	}
	lastPlayerKnownPos = ppos->getPosition();

	float distToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
	if (!isPlayerInFov(entityToChase, fov, maxChaseDistance) || distToPlayer >= maxChaseDistance + 0.5f) {
		TCompRender * cRender = get<TCompRender>();
		cRender->color = VEC4(255, 255, 0, 1);
		return BTNode::ERes::LEAVE;
	}
	else if (distToPlayer < distToAttack) {
		return BTNode::ERes::LEAVE;
	}
	else {
		rotateTowardsVec(ppos->getPosition(), dt, rotationSpeed);
		VEC3 vp = mypos->getPosition();
		VEC3 vfwd = mypos->getFront();
		vfwd.Normalize();
		vp = vp + speed * dt * vfwd;
		mypos->setPosition(vp);
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIPatrol::actionAttack(float dt)
{
	assert(arguments.find("entityToChase_actionAttack_attack") != arguments.end());
	std::string entityToChase = arguments["entityToChase_actionAttack_attack"].getString();
	
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform * ppos = player->get<TCompTransform>();

	/* TODO: always hit at the moment - change this */
	TMsgPlayerHit msg;
	msg.h_sender = CHandle(this).getOwner();
	player->sendMsg(msg);
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionResetPlayerWasSeenVariables(float dt)
{
	amountRotated = 0.f;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionGoToPlayerLastPos(float dt)
{
	assert(arguments.find("speed_actionGoToPlayerLastPos_goToPlayerLastPos") != arguments.end());
	float speed = arguments["speed_actionGoToPlayerLastPos_goToPlayerLastPos"].getFloat();
	assert(arguments.find("rotationSpeed_actionGoToPlayerLastPos_goToPlayerLastPos") != arguments.end());
	float rotationSpeed = deg2rad(arguments["rotationSpeed_actionGoToPlayerLastPos_goToPlayerLastPos"].getFloat());
	assert(arguments.find("fov_actionGoToPlayerLastPos_goToPlayerLastPos") != arguments.end());
	float fov = deg2rad(arguments["fov_actionGoToPlayerLastPos_goToPlayerLastPos"].getFloat());
	assert(arguments.find("maxChaseDistance_actionGoToPlayerLastPos_goToPlayerLastPos") != arguments.end());
	float maxChaseDistance = arguments["maxChaseDistance_actionGoToPlayerLastPos_goToPlayerLastPos"].getFloat();
	assert(arguments.find("entityToChase_actionGoToPlayerLastPos_goToPlayerLastPos") != arguments.end());
	std::string entityToChase = arguments["entityToChase_actionGoToPlayerLastPos_goToPlayerLastPos"].getString();
	
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();
	rotateTowardsVec(lastPlayerKnownPos, dt, rotationSpeed);

	isStunnedPatrolInFov(fov, maxChaseDistance);

	VEC3 vp = mypos->getPosition();

	if (VEC3::Distance(lastPlayerKnownPos, vp) < speed * dt) {
		mypos->setPosition(lastPlayerKnownPos);
		lastPlayerKnownPos = VEC3::Zero;
		return BTNode::ERes::LEAVE;
	}
	else {
		VEC3 vfwd = mypos->getFront();
		vfwd.Normalize();
		vp = vp + speed * dt * vfwd;
		mypos->setPosition(vp);
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIPatrol::actionLookForPlayer(float dt)
{
	assert(arguments.find("rotationSpeed_actionLookForPlayer_lookForPlayerSeen") != arguments.end());
	float rotationSpeed = deg2rad(arguments["rotationSpeed_actionLookForPlayer_lookForPlayerSeen"].getFloat());
	assert(arguments.find("fov_actionLookForPlayer_lookForPlayerSeen") != arguments.end());
	float fov = deg2rad(arguments["fov_actionLookForPlayer_lookForPlayerSeen"].getFloat());
	assert(arguments.find("maxRotationSeekingPlayer_actionLookForPlayer_lookForPlayerSeen") != arguments.end());
	float maxRotationSeekingPlayer = deg2rad(arguments["maxRotationSeekingPlayer_actionLookForPlayer_lookForPlayerSeen"].getFloat());
	assert(arguments.find("maxChaseDistance_actionLookForPlayer_lookForPlayerSeen") != arguments.end());
	float maxChaseDistance = arguments["maxChaseDistance_actionLookForPlayer_lookForPlayerSeen"].getFloat();
	assert(arguments.find("entityToChase_actionLookForPlayer_lookForPlayerSeen") != arguments.end());
	std::string entityToChase = arguments["entityToChase_actionLookForPlayer_lookForPlayerSeen"].getString();
	
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	isStunnedPatrolInFov(fov, maxChaseDistance);

	if (amountRotated >= maxRotationSeekingPlayer * 3) {
		suspectO_Meter = 0.f;
		TCompRender * cRender = get<TCompRender>();
		cRender->color = VEC4(1, 1, 1, 1);
		amountRotated = 0.f;
		return BTNode::ERes::LEAVE;
	}
	else {

		float y, p, r;
		mypos->getYawPitchRoll(&y, &p, &r);

		amountRotated += rotationSpeed * dt;

		if (amountRotated < maxRotationSeekingPlayer) {
			if (isLastPlayerKnownDirLeft)
			{
				y += rotationSpeed * dt;
			}
			else
			{
				y -= rotationSpeed * dt;
			}
		}
		else {
			if (isLastPlayerKnownDirLeft)
			{
				y -= rotationSpeed * dt;
			}
			else
			{
				y += rotationSpeed * dt;
			}
		}
		mypos->setYawPitchRoll(y, p, r);

		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIPatrol::actionGoToPatrol(float dt)
{
	assert(arguments.find("speed_actionGoToPatrol_goToPatrol") != arguments.end());
	float speed = arguments["speed_actionGoToPatrol_goToPatrol"].getFloat();
	assert(arguments.find("rotationSpeed_actionGoToPatrol_goToPatrol") != arguments.end());
	float rotationSpeed = deg2rad(arguments["rotationSpeed_actionGoToPatrol_goToPatrol"].getFloat());
	assert(arguments.find("distToAttack_actionGoToPatrol_goToPatrol") != arguments.end());
	float distToAttack = arguments["distToAttack_actionGoToPatrol_goToPatrol"].getFloat();
	
	TCompTransform *mypos = get<TCompTransform>();
	rotateTowardsVec(lastStunnedPatrolKnownPos, dt, rotationSpeed);

	VEC3 vp = mypos->getPosition();

	if (VEC3::Distance(vp, lastStunnedPatrolKnownPos) < distToAttack + 1.f) {
		return BTNode::ERes::LEAVE;
	}
	else {
		VEC3 vfwd = mypos->getFront();
		vfwd.Normalize();
		vp = vp + speed * dt *vfwd;
		mypos->setPosition(vp);
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIPatrol::actionFixPatrol(float dt)
{
	CHandle hPatrol = getPatrolInPos(lastStunnedPatrolKnownPos);
	if (hPatrol.isValid()) {
		CEntity* eStunnedPatrol = hPatrol;
		TMsgPatrolFixed msg;
		msg.h_sender = CHandle(this).getOwner();
		eStunnedPatrol->sendMsg(msg);
		lastStunnedPatrolKnownPos = VEC3::Zero;
	}

	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionMarkPatrolAsLost(float dt)
{
	lastStunnedPatrolKnownPos = VEC3::Zero;
	return BTNode::ERes::LEAVE;
}


/* CONDITIONS */

bool TCompAIPatrol::conditionManageStun(float dt)
{
	return hasBeenStunned;
}

bool TCompAIPatrol::conditionEndAlert(float dt)
{
	return !alarmEnded;
}

bool TCompAIPatrol::conditionShadowMerged(float dt)
{
	return hasBeenShadowMerged;
}

bool TCompAIPatrol::conditionFixed(float dt)
{
	return hasBeenFixed;
}

bool TCompAIPatrol::conditionPlayerSeen(float dt)
{
	assert(arguments.find("entityToChase_conditionPlayerSeen_managePlayerSeen") != arguments.end());
	std::string entityToChase = arguments["entityToChase_conditionPlayerSeen_managePlayerSeen"].getString();
	assert(arguments.find("fov_conditionPlayerSeen_managePlayerSeen") != arguments.end());
	float fov = deg2rad(arguments["fov_conditionPlayerSeen_managePlayerSeen"].getFloat());
	assert(arguments.find("maxChaseDistance_conditionPlayerSeen_managePlayerSeen") != arguments.end());
	float maxChaseDistance = arguments["maxChaseDistance_conditionPlayerSeen_managePlayerSeen"].getFloat();
	
	return isPlayerInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIPatrol::conditionPlayerWasSeen(float dt)
{
	return lastPlayerKnownPos != VEC3::Zero;
}

bool TCompAIPatrol::conditionPatrolSeen(float dt)
{
	assert(arguments.find("fov_conditionPatrolSeen_managePatrolSeen") != arguments.end());
	float fov = deg2rad(arguments["fov_conditionPatrolSeen_managePatrolSeen"].getFloat());
	assert(arguments.find("maxChaseDistance_conditionPatrolSeen_managePatrolSeen") != arguments.end());
	float maxChaseDistance = arguments["maxChaseDistance_conditionPatrolSeen_managePatrolSeen"].getFloat();
	
	return isStunnedPatrolInFov(fov, maxChaseDistance) || lastStunnedPatrolKnownPos != VEC3::Zero;
}

bool TCompAIPatrol::conditionFixPatrol(float dt)
{
	return isStunnedPatrolInPos(lastStunnedPatrolKnownPos);
}

bool TCompAIPatrol::conditionGoToWpt(float dt)
{
	TCompTransform* mypos = get<TCompTransform>();
	return VEC3::Distance(mypos->getPosition(), _waypoints[currentWaypoint].position) > 0.1f;
}

bool TCompAIPatrol::conditionWaitInWpt(float dt)
{
	return timerWaitingInWpt < _waypoints[currentWaypoint].minTime;
	/* TODO: manage resets if necessary */
}

bool TCompAIPatrol::conditionChase(float dt)
{
	return suspectO_Meter >= 1.f;
}

bool TCompAIPatrol::conditionPlayerAttacked(float dt)
{
	assert(arguments.find("entityToChase_conditionPlayerAttacked_managePlayerAttacked") != arguments.end());
	std::string entityToChase = arguments["entityToChase_conditionPlayerAttacked_managePlayerAttacked"].getString();
	assert(arguments.find("distToAttack_conditionPlayerAttacked_managePlayerAttacked") != arguments.end());
	float distToAttack = arguments["distToAttack_conditionPlayerAttacked_managePlayerAttacked"].getFloat();
	
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	float distToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
	return distToPlayer < distToAttack;
}

/* ASSERTS */

bool TCompAIPatrol::assertPlayerInFov(float dt)
{
	assert(arguments.find("fov_actionSuspect_suspect") != arguments.end());
	float fov = deg2rad(arguments["fov_actionSuspect_suspect"].getFloat());
	assert(arguments.find("maxChaseDistance_actionSuspect_suspect") != arguments.end());
	float maxChaseDistance = arguments["maxChaseDistance_actionSuspect_suspect"].getFloat();
	
	return isPlayerInFov("The Player", fov, maxChaseDistance);
}

bool TCompAIPatrol::assertPlayerNotInFov(float dt)
{
	assert(arguments.find("fov_actionSuspect_suspect") != arguments.end());
	float fov = deg2rad(arguments["fov_actionSuspect_suspect"].getFloat());
	assert(arguments.find("maxChaseDistance_actionSuspect_suspect") != arguments.end());
	float maxChaseDistance = arguments["maxChaseDistance_actionSuspect_suspect"].getFloat();
	
	return !isPlayerInFov("The Player", fov, maxChaseDistance);
}

bool TCompAIPatrol::assertPlayerAndPatrolNotInFov(float dt)
{
	assert(arguments.find("fov_actionSuspect_suspect") != arguments.end());
	float fov = deg2rad(arguments["fov_actionSuspect_suspect"].getFloat());
	assert(arguments.find("maxChaseDistance_actionSuspect_suspect") != arguments.end());
	float maxChaseDistance = arguments["maxChaseDistance_actionSuspect_suspect"].getFloat();
	
	return !isPlayerInFov("The Player", fov, maxChaseDistance) && !isStunnedPatrolInFov(fov, maxChaseDistance);
}

/* AUX FUNCTIONS */
void TCompAIPatrol::rotateTowardsVec(VEC3 objective, float dt, float rotationSpeed) {
	TCompTransform *mypos = get<TCompTransform>();
	float y, r, p;
	mypos->getYawPitchRoll(&y, &p, &r);
	float deltaYaw = mypos->getDeltaYawToAimTo(objective);
	if (fabsf(deltaYaw) <= rotationSpeed * dt) {
		y += deltaYaw;
	}
	else {
		if (mypos->isInLeft(objective))
		{
			y += rotationSpeed * dt;
		}
		else {
			y -= rotationSpeed * dt;
		}
	}
	mypos->setYawPitchRoll(y, p, r);
}

bool TCompAIPatrol::isPlayerInFov(std::string entityToChase, float fov, float maxChaseDistance) {

	TCompTransform *mypos = get<TCompTransform>();
	CHandle hPlayer = getEntityByName(entityToChase);
	if (hPlayer.isValid()) {
		CEntity *ePlayer = hPlayer;
		TCompTransform *ppos = ePlayer->get<TCompTransform>();

		float dist = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
		TCompPlayerController *pController = ePlayer->get<TCompPlayerController>();

		/* Player inside cone of vision */
		bool in_fov = mypos->isInFov(ppos->getPosition(), fov, deg2rad(89.f));

		return in_fov && !pController->isInShadows() && !pController->isDead() && dist <= maxChaseDistance && !isEntityHidden(hPlayer);
	}
	else {
		return false;
	}
}

bool TCompAIPatrol::isEntityHidden(CHandle hEntity)
{
	CEntity *entity = hEntity;
	TCompTransform *mypos = get<TCompTransform>();
	TCompTransform *eTransform = entity->get<TCompTransform>();
	TCompCollider *myCollider = get<TCompCollider>();
	TCompCollider *eCollider = entity->get<TCompCollider>();

	CPhysicsCapsule * capsuleCollider = (CPhysicsCapsule *)myCollider->config;

	bool isHidden = true;

	VEC3 myPosition = mypos->getPosition();
	VEC3 origin = myPosition + VEC3(0, capsuleCollider->height * 2, 0);
	VEC3 dest = VEC3::Zero;
	VEC3 dir = VEC3::Zero;

	float i = 0;
	while (isHidden && i < capsuleCollider->height * 2) {
		dest = eTransform->getPosition() + VEC3(0, Clamp(i - .1f, 0.f, capsuleCollider->height * 2), 0);
		dir = dest - origin;
		dir.Normalize();
		physx::PxRaycastHit hit;
		float dist = VEC3::Distance(origin, dest);

		//TODO: only works when behind scenery. Make the same for other enemies, dynamic objects...
		if (!EnginePhysics.Raycast(origin, dir, dist, hit, physx::PxQueryFlag::eSTATIC)) {
			isHidden = false;
		}
		i = i + (capsuleCollider->height / 2);
	}
	return isHidden;
}

void TCompAIPatrol::turnOnLight()
{
	TCompGroup* cGroup = get<TCompGroup>();
	CEntity* eCone = cGroup->getHandleByName("Cone of Light");
	TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
	cConeController->turnOnLight();
}

void TCompAIPatrol::turnOffLight() {
	TCompGroup* cGroup = get<TCompGroup>();
	CEntity* eCone = cGroup->getHandleByName("Cone of Light");
	TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
	cConeController->turnOffLight();
}

bool TCompAIPatrol::isStunnedPatrolInFov(float fov, float maxChaseDistance)
{
	std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;

	bool found = false;

	if (stunnedPatrols.size() > 0) {
		TCompTransform *mypos = get<TCompTransform>();
		for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
			TCompTransform* stunnedPatrol = ((CEntity*)stunnedPatrols[i])->get<TCompTransform>();
			if (mypos->isInFov(stunnedPatrol->getPosition(), fov, deg2rad(89.f))
				&& VEC3::Distance(mypos->getPosition(), stunnedPatrol->getPosition()) < maxChaseDistance
				&& !isEntityHidden(stunnedPatrols[i])) {
				found = true;
				lastStunnedPatrolKnownPos = stunnedPatrol->getPosition();
			}
		}
	}

	return found;
}


bool TCompAIPatrol::isStunnedPatrolInPos(VEC3 lastPos)
{
	std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;

	bool found = false;

	if (stunnedPatrols.size() > 0) {
		TCompTransform *mypos = get<TCompTransform>();
		for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
			TCompTransform* stunnedPatrol = ((CEntity*)stunnedPatrols[i])->get<TCompTransform>();
			if (stunnedPatrol->getPosition() == lastPos) {
				found = true;
			}
		}
	}

	return found;
}

CHandle TCompAIPatrol::getPatrolInPos(VEC3 lastPos)
{
	std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;

	bool found = false;
	CHandle h_stunnedPatrol;

	if (stunnedPatrols.size() > 0) {
		TCompTransform *mypos = get <TCompTransform>();
		for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
			TCompTransform* stunnedPatrol = ((CEntity*)stunnedPatrols[i])->get<TCompTransform>();
			if (stunnedPatrol->getPosition() == lastPos) {
				found = true;
				h_stunnedPatrol = stunnedPatrols[i];
			}
		}
	}

	return h_stunnedPatrol;
}

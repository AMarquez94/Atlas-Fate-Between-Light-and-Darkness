#include "mcv_platform.h"
#include "comp_bt_patrol.h"
#include "btnode.h"
#include "components/comp_name.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_controller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/object_controller/comp_cone_of_light.h"


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
	/*CHandle h(this);
	CEntity* e = h.getOwner();
	std::string aux = e->getName();*/
	
	/*createRoot("patrol", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("patrol", "manageStun", BTNode::EType::PRIORITY, (BTCondition)&TCompAIPatrol::conditionManageStun, nullptr, nullptr);
	addChild("manageStun", "shadowmerged", BTNode::EType::ACTION, (BTCondition)&TCompAIPatrol::conditionShadowMerged, (BTAction)&TCompAIPatrol::actionShadowMerged, nullptr);
	addChild("manageStun", "manageFixed", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionFixed, nullptr, nullptr);
	addChild("manageStun", "stunned", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionStunned, nullptr);
	addChild("manageFixed", "fixed", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionFixed, nullptr);
	addChild("manageFixed", "beginAlertFixed", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionBeginAlert, nullptr);
	addChild("manageFixed", "closestWptFixed", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);
	addChild("patrol", "endAlertRoot", BTNode::EType::ACTION, (BTCondition)&TCompAIPatrol::conditionEndAlert, (BTAction)&TCompAIPatrol::actionEndAlert, nullptr);
	addChild("patrol", "manageDoPatrol", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);

	addChild("manageDoPatrol", "managePlayerSeen", BTNode::EType::PRIORITY, (BTCondition)&TCompAIPatrol::conditionPlayerSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "managePlayerWasSeen", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionPlayerWasSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "managePatrolSeen", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionPatrolSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "goToWpt", BTNode::EType::ACTION, (BTCondition)&TCompAIPatrol::conditionGoToWpt, (BTAction)&TCompAIPatrol::actionGoToWpt, (BTAssert)&TCompAIPatrol::assertPlayerAndPatrolNotInFov);
	addChild("manageDoPatrol", "waitInWpt", BTNode::EType::ACTION, (BTCondition)&TCompAIPatrol::conditionWaitInWpt, (BTAction)&TCompAIPatrol::actionWaitInWpt, (BTAssert)&TCompAIPatrol::assertPlayerAndPatrolNotInFov);
	addChild("manageDoPatrol", "nextWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionNextWpt, nullptr);

	addChild("managePlayerSeen", "manageChase", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionChase, nullptr, nullptr);
	addChild("managePlayerSeen", "suspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionSuspect, nullptr);
	addChild("manageChase", "markPlayerAsSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionMarkPlayerAsSeen , nullptr);
	addChild("manageChase", "shootInhibitor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionShootInhibitor, nullptr);
	addChild("manageChase", "beginAlertChase", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionBeginAlert, nullptr);
	addChild("manageChase", "manageGoingToAttack", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("manageGoingToAttack", "managePlayerAttacked", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionPlayerAttacked, nullptr, nullptr);
	addChild("manageGoingToAttack", "chasePlayer", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionChasePlayer, nullptr);
	addChild("managePlayerAttacked", "attack", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionAttack, nullptr);
	addChild("managePlayerAttacked", "endAlertAttacked", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionEndAlert, nullptr);
	addChild("managePlayerAttacked", "closestWptAttacked", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);

	addChild("managePlayerWasSeen", "resetPlayerWasSeenVariables", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionResetPlayerWasSeenVariables, nullptr);
	addChild("managePlayerWasSeen", "goToPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionGoToPlayerLastPos, (BTAssert)&TCompAIPatrol::assertPlayerNotInFov);
	addChild("managePlayerWasSeen", "lookForPlayerSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionLookForPlayer, (BTAssert)&TCompAIPatrol::assertPlayerNotInFov);
	addChild("managePlayerWasSeen", "closestWptPlayerSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);

	addChild("managePatrolSeen", "goToPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionGoToPatrol, (BTAssert)&TCompAIPatrol::assertPlayerNotInFov);
	addChild("managePatrolSeen", "manageInPatrolPos", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);

	addChild("manageInPatrolPos", "manageFixPatrol", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionFixPatrol, nullptr, nullptr);
	addChild("manageFixPatrol", "fixPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionFixPatrol, nullptr);
	addChild("manageFixPatrol", "beginAlertPatrolSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionBeginAlert, nullptr);
	addChild("manageFixPatrol", "closestWptPatrolSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);

	addChild("manageInPatrolPos", "managePatrolLost", BTNode::EType::SEQUENCE, nullptr, nullptr, nullptr);
	addChild("managePatrolLost", "markPatrolAsLost", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionMarkPatrolAsLost, nullptr);
	addChild("managePatrolLost", "lookForPlayerPatrolWasSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionLookForPlayer, (BTAssert)&TCompAIPatrol::assertPlayerAndPatrolNotInFov);
	addChild("managePatrolLost", "closestWptPatrolWasSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);*/

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

	rotationSpeed = deg2rad(rotationSpeedDeg);
	fov = deg2rad(fovDeg);
	maxRotationSeekingPlayer = deg2rad(maxRotationSeekingPlayerDeg);
	startLightsOn = j.value("startLightsOn", false);
	currentWaypoint = 0;
	/* TODO: ¿Init node? */
}

void TCompAIPatrol::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
	TCompName *tName = get<TCompName>();
	name = tName->getName();

	if (_waypoints.size() == 0) {
		Waypoint wpt;
		wpt.position = ((TCompTransform*)get<TCompTransform>())->getPosition();
		wpt.lookAt = ((TCompTransform*)get<TCompTransform>())->getFront();
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
	for (int i = 0; i <stunnedPatrols.size() && !found; i++) {
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
	TCompTransform *mypos = get<TCompTransform>();
	rotateTowardsVec(getWaypoint().position, dt);

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
	if (timerWaitingInWpt >= getWaypoint().minTime) {
		return BTNode::ERes::LEAVE;
	}
	else {
		timerWaitingInWpt += dt;
		TCompTransform *mypos = get<TCompTransform>();
		rotateTowardsVec(mypos->getPosition() + getWaypoint().lookAt, dt);
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
	TCompRender *cRender = get<TCompRender>();
	cRender->color = VEC4(255, 255, 0, 1);
	// chase
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	/* Distance to player */
	float distanceToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());

	if (distanceToPlayer <= autoChaseDistance && isPlayerInFov()) {
		suspectO_Meter = 1.f;
		rotateTowardsVec(ppos->getPosition(), dt);
	}
	else if (distanceToPlayer <= maxChaseDistance && isPlayerInFov()) {
		suspectO_Meter = Clamp(suspectO_Meter + dt * incrBaseSuspectO_Meter, 0.f, 1.f);							//TODO: increment more depending distance and noise
		rotateTowardsVec(ppos->getPosition(), dt);
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
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform * ppos = player->get<TCompTransform>();
	lastPlayerKnownPos = ppos->getPosition();
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionShootInhibitor(float dt)
{
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
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	isStunnedPatrolInFov();

	if (lastPlayerKnownPos != VEC3::Zero) {

		/* If we had the player's previous position, know where he is going */
		isLastPlayerKnownDirLeft = mypos->isInLeft(ppos->getPosition() - lastPlayerKnownPos);
	}
	lastPlayerKnownPos = ppos->getPosition();

	float distToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
	if (!isPlayerInFov() || distToPlayer >= maxChaseDistance + 0.5f) {
		TCompRender * cRender = get<TCompRender>();
		cRender->color = VEC4(255, 255, 0, 1);
		return BTNode::ERes::LEAVE;
	}
	else if (distToPlayer < distToAttack) {
		return BTNode::ERes::LEAVE;
	}
	else {
		rotateTowardsVec(ppos->getPosition(), dt);
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
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();
	rotateTowardsVec(lastPlayerKnownPos, dt);

	isStunnedPatrolInFov();

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
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	isStunnedPatrolInFov();

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
	TCompTransform *mypos = get<TCompTransform>();
	rotateTowardsVec(lastStunnedPatrolKnownPos, dt);

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
	return isPlayerInFov();
}

bool TCompAIPatrol::conditionPlayerWasSeen(float dt)
{
	return lastPlayerKnownPos != VEC3::Zero;
}

bool TCompAIPatrol::conditionPatrolSeen(float dt)
{
	return isStunnedPatrolInFov() || lastStunnedPatrolKnownPos != VEC3::Zero;
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
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	float distToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
	return distToPlayer < distToAttack;
}

/* ASSERTS */

bool TCompAIPatrol::assertPlayerInFov(float dt)
{
	return isPlayerInFov();
}

bool TCompAIPatrol::assertPlayerNotInFov(float dt)
{
	return !isPlayerInFov();
}

bool TCompAIPatrol::assertPlayerAndPatrolNotInFov(float dt)
{
	return !isPlayerInFov() && !isStunnedPatrolInFov();
}

/* AUX FUNCTIONS */
void TCompAIPatrol::rotateTowardsVec(VEC3 objective, float dt){
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

bool TCompAIPatrol::isPlayerInFov() {

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

	bool isHidden = true;

	VEC3 myPosition = mypos->getPosition();
	VEC3 origin = myPosition + VEC3(0, myCollider->config.height * 2, 0);
	VEC3 dest = VEC3::Zero;
	VEC3 dir = VEC3::Zero;

	float i = 0;
	while (isHidden && i < eCollider->config.height * 2) {
		dest = eTransform->getPosition() + VEC3(0, Clamp(i - .1f, 0.f, eCollider->config.height * 2), 0);
		dir = dest - origin;
		dir.Normalize();
		physx::PxRaycastHit hit;
		float dist = VEC3::Distance(origin, dest);

		//TODO: only works when behind scenery. Make the same for other enemies, dynamic objects...
		if (!EnginePhysics.Raycast(origin, dir, dist, hit, physx::PxQueryFlag::eSTATIC)) {
			isHidden = false;
		}
		i = i + (eCollider->config.height / 2);
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

bool TCompAIPatrol::isStunnedPatrolInFov()
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

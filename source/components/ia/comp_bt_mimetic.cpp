#include "mcv_platform.h"
#include "comp_bt_mimetic.h"
#include "btnode.h"
#include "components/comp_name.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_controller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/object_controller/comp_cone_of_light.h"


DECL_OBJ_MANAGER("ai_mimetic", TCompAIMimetic);

void TCompAIMimetic::debugInMenu() {

	TCompIAController::debugInMenu();
	
	if (current) {
		validState = current->getName();
	}
	
	ImGui::Text("Current state: %s", validState.c_str());
}

void TCompAIMimetic::load(const json& j, TEntityParseContext& ctx) {

	loadActions();
	loadConditions();
	loadAsserts();
	//TCompIAController::loadTree(j);
	
	createRoot("mimetic", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("mimetic", "stunned", BTNode::EType::ACTION, (BTCondition)&TCompAIMimetic::conditionHasBeenStunned, (BTAction)&TCompAIMimetic::actionStunned, nullptr);
	
	addChild("mimetic", "manageInactive", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsNotActive, nullptr, nullptr);
	addChild("manageInactive", "manageInactiveTypeSleep", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsSlept, nullptr, nullptr);
	addChild("manageInactive", "manageInactiveTypeWall", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsTypeWall, nullptr, nullptr);
	addChild("manageInactive", "manageInactiveTypeFloor", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsTypeFloor, nullptr, nullptr);

	addChild("manageInactiveTypeSleep", "sleep", BTNode::EType::ACTION, (BTCondition)&TCompAIMimetic::conditionNotListenedNoise, (BTAction)&TCompAIMimetic::actionSleep, nullptr);
	addChild("manageInactiveTypeSleep", "wakeUp", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWakeUp, nullptr);

	addChild("manageInactiveTypeWall", "observeTypeWall", BTNode::EType::ACTION, (BTCondition)&TCompAIMimetic::conditionIsNotPlayerInFov, (BTAction)&TCompAIMimetic::actionObserve, nullptr);
	addChild("manageInactiveTypeWall", "manageSetActiveTypeWall", BTNode::EType::SEQUENCE, nullptr, nullptr, nullptr);
	addChild("manageSetActiveTypeWall", "setActiveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetActive, nullptr);
	addChild("manageSetActiveTypeWall", "jumpFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionJumpFloor, nullptr);

	addChild("manageInactiveTypeFloor", "manageObserve", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsNotPlayerInFov, nullptr, nullptr);
	addChild("manageInactiveTypeFloor", "setActiveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetActive, nullptr);
	addChild("manageObserve", "observeTypeFloor", BTNode::EType::ACTION, (BTCondition)&TCompAIMimetic::conditionHasNotWaypoints, (BTAction)&TCompAIMimetic::actionObserve, nullptr);
	addChild("manageObserve", "managePatrol", BTNode::EType::SEQUENCE, nullptr, nullptr, nullptr);
	addChild("managePatrol", "goToWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToWpt, (BTAssert)&TCompAIMimetic::assertNotPlayerInFov);
	addChild("managePatrol", "resetTimerWaitInWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetTimerWaiting, nullptr);
	addChild("managePatrol", "waitInWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitInWpt, (BTAssert)&TCompAIMimetic::assertNotPlayerInFov);
	addChild("managePatrol", "nextWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionNextWpt, nullptr);

	addChild("mimetic", "manageSuspect", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionNotSurePlayerInFov, nullptr, nullptr);
	addChild("manageSuspect", "suspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSuspect, nullptr);

	addChild("mimetic", "manageChase", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionPlayerSeenForSure, nullptr, nullptr);
	addChild("manageChase", "chasePlayerWithNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionChasePlayerWithNoise, nullptr);

	addChild("mimetic", "managePlayerLost", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionNotGoingInactive, nullptr, nullptr);
	addChild("managePlayerLost", "goToPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToPlayerLastPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFov);
	addChild("managePlayerLost", "resetTimerWaitInPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetTimerWaiting, nullptr);
	addChild("managePlayerLost", "waitInPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitInPlayerLastPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFov);
	addChild("managePlayerLost", "setGoInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetGoInactive, (BTAssert)&TCompAIMimetic::assertNotPlayerInFov);

	addChild("mimetic", "manageGoingInactive", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("manageGoingInactive", "manageGoingInactiveTypeWall", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionIsTypeWall, nullptr, nullptr);
	addChild("manageGoingInactiveTypeWall", "goToInitialPosTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFov);
	addChild("manageGoingInactiveTypeWall", "jumpWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionJumpWall, nullptr);
	addChild("manageGoingInactiveTypeWall", "setInactiveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetInactive, nullptr);
	addChild("manageGoingInactive", "manageGoingInactiveTypeFloor", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionIsTypeFloor, nullptr, nullptr);
	addChild("manageGoingInactiveTypeFloor", "goToInitialPosTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFov);
	addChild("manageGoingInactiveTypeFloor", "setInactiveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetInactive, nullptr);

	/*createRoot("patrol", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("patrol", "manageStun", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionManageStun, nullptr, nullptr);
	addChild("manageStun", "shadowmerged", BTNode::EType::ACTION, (BTCondition)&TCompAIMimetic::conditionShadowMerged, (BTAction)&TCompAIMimetic::actionShadowMerged, nullptr);
	addChild("manageStun", "manageFixed", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionFixed, nullptr, nullptr);
	addChild("manageStun", "stunned", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionStunned, nullptr);
	addChild("manageFixed", "fixed", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionFixed, nullptr);
	addChild("manageFixed", "beginAlertFixed", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionBeginAlert, nullptr);
	addChild("manageFixed", "closestWptFixed", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionClosestWpt, nullptr);
	addChild("patrol", "endAlertRoot", BTNode::EType::ACTION, (BTCondition)&TCompAIMimetic::conditionEndAlert, (BTAction)&TCompAIMimetic::actionEndAlert, nullptr);
	addChild("patrol", "manageDoPatrol", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);

	addChild("manageDoPatrol", "managePlayerSeen", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionPlayerSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "managePlayerWasSeen", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionPlayerWasSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "managePatrolSeen", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionPatrolSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "goToWpt", BTNode::EType::ACTION, (BTCondition)&TCompAIMimetic::conditionGoToWpt, (BTAction)&TCompAIMimetic::actionGoToWpt, (BTAssert)&TCompAIMimetic::assertPlayerAndPatrolNotInFov);
	addChild("manageDoPatrol", "waitInWpt", BTNode::EType::ACTION, (BTCondition)&TCompAIMimetic::conditionWaitInWpt, (BTAction)&TCompAIMimetic::actionWaitInWpt, (BTAssert)&TCompAIMimetic::assertPlayerAndPatrolNotInFov);
	addChild("manageDoPatrol", "nextWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionNextWpt, nullptr);

	addChild("managePlayerSeen", "manageChase", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionChase, nullptr, nullptr);
	addChild("managePlayerSeen", "suspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSuspect, nullptr);
	addChild("manageChase", "markPlayerAsSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionMarkPlayerAsSeen , nullptr);
	addChild("manageChase", "shootInhibitor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionShootInhibitor, nullptr);
	addChild("manageChase", "beginAlertChase", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionBeginAlert, nullptr);
	addChild("manageChase", "manageGoingToAttack", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("manageGoingToAttack", "managePlayerAttacked", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionPlayerAttacked, nullptr, nullptr);
	addChild("manageGoingToAttack", "chasePlayer", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionChasePlayer, nullptr);
	addChild("managePlayerAttacked", "attack", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionAttack, nullptr);
	addChild("managePlayerAttacked", "endAlertAttacked", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionEndAlert, nullptr);
	addChild("managePlayerAttacked", "closestWptAttacked", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionClosestWpt, nullptr);

	addChild("managePlayerWasSeen", "resetPlayerWasSeenVariables", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetPlayerWasSeenVariables, nullptr);
	addChild("managePlayerWasSeen", "goToPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToPlayerLastPos, (BTAssert)&TCompAIMimetic::assertPlayerNotInFov);
	addChild("managePlayerWasSeen", "lookForPlayerSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionLookForPlayer, (BTAssert)&TCompAIMimetic::assertPlayerNotInFov);
	addChild("managePlayerWasSeen", "closestWptPlayerSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionClosestWpt, nullptr);

	addChild("managePatrolSeen", "goToPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToPatrol, (BTAssert)&TCompAIMimetic::assertPlayerNotInFov);
	addChild("managePatrolSeen", "manageInPatrolPos", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);

	addChild("manageInPatrolPos", "manageFixPatrol", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionFixPatrol, nullptr, nullptr);
	addChild("manageFixPatrol", "fixPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionFixPatrol, nullptr);
	addChild("manageFixPatrol", "beginAlertPatrolSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionBeginAlert, nullptr);
	addChild("manageFixPatrol", "closestWptPatrolSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionClosestWpt, nullptr);

	addChild("manageInPatrolPos", "managePatrolLost", BTNode::EType::SEQUENCE, nullptr, nullptr, nullptr);
	addChild("managePatrolLost", "markPatrolAsLost", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionMarkPatrolAsLost, nullptr);
	addChild("managePatrolLost", "lookForPlayerPatrolWasSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionLookForPlayer, (BTAssert)&TCompAIMimetic::assertPlayerAndPatrolNotInFov);
	addChild("managePatrolLost", "closestWptPatrolWasSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionClosestWpt, nullptr);*/

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

	type = EType::FLOOR;
	rotationSpeed = deg2rad(rotationSpeedDeg);
	fov = deg2rad(fovDeg);
	startLightsOn = j.value("startLightsOn", false);
	currentWaypoint = 0;
	/* TODO: ¿Init node? */
}

void TCompAIMimetic::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
	TCompName *tName = get<TCompName>();
	name = tName->getName();

	if (startLightsOn) {
		turnOnLight();
	}

	TCompTransform *tTransform = get<TCompTransform>();
	initialLookAt = tTransform->getFront();
	initialPos = tTransform->getPosition();
}

void TCompAIMimetic::onMsgPlayerDead(const TMsgPlayerDead& msg) {

	alarmEnded = false;
}

void TCompAIMimetic::onMsgMimeticStunned(const TMsgEnemyStunned & msg)
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

	//TCompGroup* cGroup = get<TCompGroup>();
	//CEntity* eCone = cGroup->getHandleByName("Cone of Vision");
	//TCompRender * coneRender = eCone->get<TCompRender>();
	//coneRender->visible = false;

	lastPlayerKnownPos = VEC3::Zero;

	current = nullptr;
}

void TCompAIMimetic::registerMsgs()
{
	DECL_MSG(TCompAIMimetic, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompAIMimetic, TMsgPlayerDead, onMsgPlayerDead);
	DECL_MSG(TCompAIMimetic, TMsgEnemyStunned, onMsgMimeticStunned);
}

void TCompAIMimetic::loadActions() {
	actions_initializer.clear();
	
}

void TCompAIMimetic::loadConditions() {
	conditions_initializer.clear();

}

void TCompAIMimetic::loadAsserts() {
	asserts_initializer.clear();

}

/* ACTIONS */

BTNode::ERes TCompAIMimetic::actionStunned(float dt)
{
	return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIMimetic::actionObserve(float dt)
{
	if (isPlayerInFov()) {
		amountRotatedObserving = 0.f;
		return BTNode::ERes::LEAVE;
	}
	else {
		TCompTransform * myPos = get<TCompTransform>();
		if (amountRotatedObserving >= maxAmountRotateObserving * 3) {
			amountRotatedObserving = 0.f;
		}
		else {
			float y, p, r;
			myPos->getYawPitchRoll(&y, &p, &r);
			amountRotatedObserving += rotationSpeed * dt;
			float newYaw = amountRotatedObserving < maxAmountRotateObserving ? rotationSpeed * dt : -rotationSpeed * dt;
			y += newYaw;
			myPos->setYawPitchRoll(y, p, r);
		}
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIMimetic::actionSetActive(float dt)
{
	isActive = true;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionJumpFloor(float dt)
{
	return BTNode::ERes::UNIMPLEMENTED; //TODO implement
}

BTNode::ERes TCompAIMimetic::actionGoToWpt(float dt)
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

BTNode::ERes TCompAIMimetic::actionResetTimerWaiting(float dt)
{
	timerWaitingInWpt = 0.f;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionWaitInWpt(float dt)
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

BTNode::ERes TCompAIMimetic::actionNextWpt(float dt)
{
	timerWaitingInWpt = 0.f;
	currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionSleep(float dt)
{
	return BTNode::STAY;
}

BTNode::ERes TCompAIMimetic::actionWakeUp(float dt)
{
	isSlept = false;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionSuspect(float dt)
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

BTNode::ERes TCompAIMimetic::actionChasePlayerWithNoise(float dt)
{
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

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

BTNode::ERes TCompAIMimetic::actionGoToPlayerLastPos(float dt)
{
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();
	rotateTowardsVec(lastPlayerKnownPos, dt);

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

BTNode::ERes TCompAIMimetic::actionWaitInPlayerLastPos(float dt)
{
	timerWaitingInWpt += dt;
	if (timerWaitingInWpt < 5.f /*TODO: not hardcoded*/) {
		return BTNode::STAY;
	}
	else {
		return BTNode::ERes::LEAVE;
	}
}

BTNode::ERes TCompAIMimetic::actionSetGoInactive(float dt)
{
	goingInactive = true;
	suspectO_Meter = 0.f;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionGoToInitialPos(float dt)
{
	TCompTransform *mypos = get<TCompTransform>();
	rotateTowardsVec(initialPos, dt);

	VEC3 vp = mypos->getPosition();

	if (VEC3::Distance(initialPos, vp) < speed * dt) {
		mypos->setPosition(initialPos);
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

BTNode::ERes TCompAIMimetic::actionJumpWall(float dt)
{
	return BTNode::ERes::UNIMPLEMENTED;
}

BTNode::ERes TCompAIMimetic::actionSetInactive(float dt)
{
	isActive = false;
	goingInactive = false;
	return BTNode::ERes::LEAVE;
}


/* CONDITIONS */

bool TCompAIMimetic::conditionHasBeenStunned(float dt)
{
	return hasBeenStunned;
}

bool TCompAIMimetic::conditionIsTypeWall(float dt)
{
	return type == EType::WALL;
}

bool TCompAIMimetic::conditionIsNotPlayerInFov(float dt)
{
	return !isPlayerInFov();
}

bool TCompAIMimetic::conditionIsNotActive(float dt)
{
	return !isActive;
}

bool TCompAIMimetic::conditionIsTypeFloor(float dt)
{
	return type == EType::FLOOR;
}

bool TCompAIMimetic::conditionIsSlept(float dt)
{
	return isSlept;
}

bool TCompAIMimetic::conditionHasNotWaypoints(float dt)
{
	return _waypoints.size() == 0;
}

bool TCompAIMimetic::conditionNotListenedNoise(float dt)
{
	return false;
}

bool TCompAIMimetic::conditionNotSurePlayerInFov(float dt)
{
	return suspectO_Meter < 1.f && (suspectO_Meter > 0.f || isPlayerInFov());
}

bool TCompAIMimetic::conditionPlayerSeenForSure(float dt)
{
	return isPlayerInFov() && suspectO_Meter >= 1.f;
}

bool TCompAIMimetic::conditionIsPlayerInFov(float dt)
{
	return isPlayerInFov();
}

bool TCompAIMimetic::conditionNotGoingInactive(float dt)
{
	return !goingInactive;
}

/* ASSERTS */

bool TCompAIMimetic::assertNotPlayerInFov(float dt)
{
	return !isPlayerInFov();
}

/* AUX FUNCTIONS */
void TCompAIMimetic::rotateTowardsVec(VEC3 objective, float dt){
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

bool TCompAIMimetic::isPlayerInFov() {

	TCompTransform *mypos = get<TCompTransform>();
	CHandle hPlayer = getEntityByName(entityToChase);
	if (hPlayer.isValid()) {
		CEntity *ePlayer = hPlayer;
		TCompTransform *ppos = ePlayer->get<TCompTransform>();

		float dist = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
		TCompPlayerController *pController = ePlayer->get<TCompPlayerController>();
		
		/* Player inside cone of vision */
		bool in_fov = mypos->isInFov(ppos->getPosition(), fov);

		return in_fov && !pController->isInShadows() && !pController->isDead() && dist <= maxChaseDistance && !isEntityHidden(hPlayer);
	}
	else {
		return false;
	}
}

bool TCompAIMimetic::isEntityHidden(CHandle hEntity)
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

void TCompAIMimetic::turnOnLight()
{
	TCompGroup* cGroup = get<TCompGroup>();
	CEntity* eCone = cGroup->getHandleByName("Cone of Light");
	TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
	cConeController->turnOnLight();
}

void TCompAIMimetic::turnOffLight() {
	TCompGroup* cGroup = get<TCompGroup>();
	CEntity* eCone = cGroup->getHandleByName("Cone of Light");
	TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
	cConeController->turnOffLight();
}

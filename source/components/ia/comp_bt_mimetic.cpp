#include "mcv_platform.h"
#include "comp_bt_mimetic.h"
#include "btnode.h"
#include "components/comp_name.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/object_controller/comp_cone_of_light.h"
#include "geometry/angular.h"
#include "components/physics/comp_rigidbody.h"
#include "components/physics/comp_collider.h"
#include "physics/physics_collider.h"
#include "render/render_utils.h"


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
	addChild("mimetic", "manageStun", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionHasBeenStunned, nullptr, nullptr);
	addChild("manageStun", "stunned", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionStunned, nullptr);
	addChild("manageStun", "exploded", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionExplode, nullptr);
	
	addChild("mimetic", "manageInactive", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsNotActive, nullptr, nullptr);
	addChild("manageInactive", "manageInactiveTypeSleep", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsSlept, nullptr, nullptr);
	addChild("manageInactive", "manageInactiveTypeWall", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsTypeWall, nullptr, nullptr);
	addChild("manageInactive", "manageInactiveTypeFloor", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsTypeFloor, nullptr, nullptr);

	addChild("manageInactiveTypeSleep", "sleep", BTNode::EType::ACTION, (BTCondition)&TCompAIMimetic::conditionNotListenedNoise, (BTAction)&TCompAIMimetic::actionSleep, nullptr);
	addChild("manageInactiveTypeSleep", "wakeUp", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWakeUp, nullptr);

	addChild("manageInactiveTypeWall", "manageObserveTypeWall", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionIsNotPlayerInFovAndNotNoise, nullptr, nullptr);
	addChild("manageInactiveTypeWall", "setActiveTypewall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetActive, nullptr);

	addChild("manageObserveTypeWall", "resetVariablesObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetObserveVariables, nullptr);
	addChild("manageObserveTypeWall", "turnLeftObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionObserveLeft, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageObserveTypeWall", "waitLeftObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitObserving, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageObserveTypeWall", "turnRightObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionObserveRight, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageObserveTypeWall", "waitRightObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitObserving, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);

	addChild("manageInactiveTypeFloor", "manageInactiveBehaviour", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsNotPlayerInFovAndNotNoise, nullptr, nullptr);
	addChild("manageInactiveTypeFloor", "setActiveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetActive, nullptr);
	addChild("manageInactiveBehaviour", "manageObserveTypeFloor", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionHasNotWaypoints, nullptr, nullptr);
	addChild("manageInactiveBehaviour", "managePatrol", BTNode::EType::SEQUENCE, nullptr, nullptr, nullptr);
	
	addChild("manageObserveTypeFloor", "resetVariablesObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetObserveVariables, nullptr);
	addChild("manageObserveTypeFloor", "turnLeftObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionObserveLeft, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageObserveTypeFloor", "waitLeftObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitObserving, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageObserveTypeFloor", "turnRightObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionObserveRight, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageObserveTypeFloor", "waitRightObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitObserving, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	
	addChild("managePatrol", "goToWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToWpt, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("managePatrol", "resetTimerWaitInWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetTimerWaiting, nullptr);
	addChild("managePatrol", "waitInWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitInWpt, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("managePatrol", "nextWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionNextWpt, nullptr);

	addChild("mimetic", "manageChase", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionPlayerSeenForSure, nullptr, nullptr);
	addChild("manageChase", "jumpFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionJumpFloor, nullptr);
	addChild("manageChase", "resetVariablesChase", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetVariablesChase, nullptr);
	addChild("manageChase", "chasePlayerWithNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionChasePlayerWithNoise, nullptr);

	addChild("mimetic", "manageArtificialNoise", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionHasHeardArtificialNoise, nullptr, nullptr);
	addChild("manageArtificialNoise", "markArtificialNoiseAsInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionMarkNoiseAsInactive, nullptr);
	addChild("manageArtificialNoise", "jumpFloorNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionJumpFloor, nullptr);
	addChild("manageArtificialNoise", "goToNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToNoiseSource, nullptr/*(BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise*/);
	addChild("manageArtificialNoise", "waitInNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitInNoiseSource, nullptr/*(BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise*/);

	addChild("mimetic", "manageSuspect", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionNotSurePlayerInFov, nullptr, nullptr);
	addChild("manageSuspect", "suspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSuspect, nullptr);
	addChild("manageSuspect", "rotateToInitialPosSuspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionRotateToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);

	addChild("mimetic", "manageNaturalNoise", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionHasHeardNaturalNoise, nullptr, nullptr);
	addChild("manageNaturalNoise", "markNaturalNoiseAsInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionMarkNoiseAsInactive, nullptr);
	addChild("manageNaturalNoise", "rotateToNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionRotateToNoiseSource, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageNaturalNoise", "setGoInactiveAfterNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetGoInactive, nullptr);

	addChild("mimetic", "managePlayerLost", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionNotGoingInactive, nullptr, nullptr);
	addChild("managePlayerLost", "goToPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToPlayerLastPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("managePlayerLost", "resetTimerWaitInPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetTimerWaiting, nullptr);
	addChild("managePlayerLost", "waitInPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitInPlayerLastPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("managePlayerLost", "setGoInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetGoInactive, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);

	addChild("mimetic", "manageGoingInactive", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("manageGoingInactive", "manageGoingInactiveTypeWall", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionIsTypeWall, nullptr, nullptr);
	addChild("manageGoingInactiveTypeWall", "goToInitialPosTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageGoingInactiveTypeWall", "rotateToInitialPosTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionRotateToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageGoingInactiveTypeWall", "jumpWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionJumpWall, nullptr);
	addChild("manageGoingInactiveTypeWall", "holdOnWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionHoldOnWall, nullptr);
	addChild("manageGoingInactiveTypeWall", "setInactiveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetInactive, nullptr);
	addChild("manageGoingInactive", "manageGoingInactiveTypeFloor", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionIsTypeFloor, nullptr, nullptr);
	addChild("manageGoingInactiveTypeFloor", "goToInitialPosTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageGoingInactiveTypeFloor", "rotateToInitialPosTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionRotateToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
	addChild("manageGoingInactiveTypeFloor", "setInactiveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetInactive, nullptr);

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

	type = parseStringMimeticType(j.value("type", ""));
	rotationSpeedChase = deg2rad(rotationSpeedChaseDeg);
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

	if (type == EType::WALL) {
		setGravityToFaceWall();
	}
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

	TCompGroup* cGroup = get<TCompGroup>();
	CEntity* eCone = cGroup->getHandleByName("Cone of Vision");
	TCompRender * coneRender = eCone->get<TCompRender>();
	coneRender->visible = false;

	lastPlayerKnownPos = VEC3::Zero;

	current = nullptr;
}

void TCompAIMimetic::onMsgNoiseListened(const TMsgNoiseMade & msg)
{
	if (!msg.isArtificialNoise && !isPlayerInFov()) {
		hasHeardNaturalNoise = true;
		noiseSource = msg.noiseOrigin;
	}
	else {
		if (msg.isArtificialNoise) {
			hasHeardArtificialNoise = true;
		}
		else {
			hasHeardNaturalNoise = true;
		}
		noiseSource = msg.noiseOrigin;
	}
}

void TCompAIMimetic::registerMsgs()
{
	DECL_MSG(TCompAIMimetic, TMsgScenePaused, onMsgScenePaused);
	DECL_MSG(TCompAIMimetic, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompAIMimetic, TMsgPlayerDead, onMsgPlayerDead);
	DECL_MSG(TCompAIMimetic, TMsgEnemyStunned, onMsgMimeticStunned);
	DECL_MSG(TCompAIMimetic, TMsgNoiseMade, onMsgNoiseListened);
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
	timerToExplode += dt;

	if (timerToExplode > 1.f) {
		return BTNode::ERes::LEAVE;
	}
	else {
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIMimetic::actionExplode(float dt)
{
	CHandle(this).getOwner().destroy();
	return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIMimetic::actionResetObserveVariables(float dt)
{
	timerWaitingInObservation = 0.f;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionObserveLeft(float dt)
{
	if (isPlayerInFov()) {
		return BTNode::ERes::LEAVE;
	}
	else {
		VEC3 objective = rotateVectorAroundAxis(initialLookAt, VEC3(0, 1.f, 0), maxAmountRotateObserving);
		TCompTransform *mypos = get<TCompTransform>();
		if (rotateTowardsVec(mypos->getPosition() + objective, rotationSpeedObservation, dt)) {
			return BTNode::ERes::LEAVE;
		}
		else {
			return BTNode::ERes::STAY;
		}
	}
}

BTNode::ERes TCompAIMimetic::actionObserveRight(float dt)
{
	if (isPlayerInFov()) {
		return BTNode::ERes::LEAVE;
	}
	else {
		VEC3 objective = rotateVectorAroundAxis(initialLookAt, VEC3(0, 1, 0), -maxAmountRotateObserving);
		TCompTransform *mypos = get<TCompTransform>();
		if (rotateTowardsVec(mypos->getPosition() + objective, rotationSpeedObservation, dt)) {
			return BTNode::ERes::LEAVE;
		}
		else {
			return BTNode::ERes::STAY;
		}
	}
}

BTNode::ERes TCompAIMimetic::actionWaitObserving(float dt)
{
	timerWaitingInObservation += dt;

	if (timerWaitingInObservation < 2.f) {
		return BTNode::ERes::STAY;
	}
	else {
		timerWaitingInObservation = 0.f;
		return BTNode::ERes::LEAVE;
	}
}
//
//BTNode::ERes TCompAIMimetic::actionObserve(float dt)
//{
//	if (isPlayerInFov()) {
//		amountRotatedObserving = 0.f;
//		return BTNode::ERes::LEAVE;
//	}
//	else {
//		TCompTransform * myPos = get<TCompTransform>();
//		if (amountRotatedObserving >= maxAmountRotateObserving * 4) {
//			amountRotatedObserving = 0.f;
//		}
//		else {
//			float y, p, r;
//			myPos->getYawPitchRoll(&y, &p, &r);
//			amountRotatedObserving += rotationSpeedObservation * dt;
//			float newYaw = amountRotatedObserving < maxAmountRotateObserving || amountRotatedObserving > maxAmountRotateObserving * 3 ? rotationSpeedObservation * dt : -rotationSpeedObservation * dt;
//			y += newYaw;
//			myPos->setYawPitchRoll(y, p, r);
//		}
//		return BTNode::ERes::STAY;
//	}
//}

BTNode::ERes TCompAIMimetic::actionSetActive(float dt)
{
	isActive = true;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionJumpFloor(float dt)
{
	TCompRigidbody *tCollider = get<TCompRigidbody>();

	tCollider->setNormalGravity(VEC3(0,-9.8f,0));
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionGoToWpt(float dt)
{
	TCompTransform *mypos = get<TCompTransform>();
	rotateTowardsVec(getWaypoint().position, rotationSpeedObservation, dt);

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
		rotateTowardsVec(mypos->getPosition() + getWaypoint().lookAt, rotationSpeedObservation, dt);
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
		cRender->color = VEC4(255, 0, 0, 1);
		rotateTowardsVec(ppos->getPosition(), rotationSpeedObservation, dt);
	}
	else if (distanceToPlayer <= maxChaseDistance && isPlayerInFov()) {
		suspectO_Meter = Clamp(suspectO_Meter + dt * incrBaseSuspectO_Meter, 0.f, 1.f);							//TODO: increment more depending distance and noise
		rotateTowardsVec(ppos->getPosition(), rotationSpeedObservation, dt);
	}
	else {
		suspectO_Meter = Clamp(suspectO_Meter - dt * dcrSuspectO_Meter, 0.f, 1.f);
	}

	if (suspectO_Meter <= 0.f || suspectO_Meter >= 1.f) {
		if (suspectO_Meter <= 0) {
			isActive = false;
			cRender->color = VEC4(1, 1, 1, 1);
		}
		else {
			cRender->color = VEC4(255, 0, 0, 1);
		}
		return BTNode::ERes::LEAVE;
	}
	else {
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIMimetic::actionRotateToNoiseSource(float dt)
{
	TCompTransform *myPos = get<TCompTransform>();
	bool isInObjective = rotateTowardsVec(noiseSource, rotationSpeedNoise, dt);
	return isInObjective ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIMimetic::actionResetVariablesChase(float dt)
{
	goingInactive = false;
	isActive = true;
	hasHeardArtificialNoise = false;
	hasHeardNaturalNoise = false;

	TMsgMakeNoise msg;
	msg.isArtificial = true;
	msg.isNoise = true;
	msg.isOnlyOnce = false;
	msg.timeToRepeat = 1.f;
	msg.noiseRadius = 20.f;
	TCompGroup * tGroup = get<TCompGroup>();
	if (tGroup) {
		CEntity * eNoiseEmitter = tGroup->getHandleByName("Noise Emitter");
		eNoiseEmitter->sendMsg(msg);
	}

	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionChasePlayerWithNoise(float dt)
{
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();
	TCompRender * cRender = get<TCompRender>();

	cRender->color = VEC4(255, 0, 0, 1);

	if (lastPlayerKnownPos != VEC3::Zero) {

		/* If we had the player's previous position, know where he is going */
		isLastPlayerKnownDirLeft = mypos->isInLeft(ppos->getPosition() - lastPlayerKnownPos);
	}
	lastPlayerKnownPos = ppos->getPosition();

	float distToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
	if (!isPlayerInFov() || distToPlayer >= maxChaseDistance + 0.5f) {
		cRender->color = VEC4(255, 255, 0, 1);
		TMsgMakeNoise msg;
		msg.isArtificial = true;
		msg.isNoise = false;
		msg.isOnlyOnce = false;
		msg.timeToRepeat = 1.f;
		msg.noiseRadius = 0.01f;
		TCompGroup * tGroup = get<TCompGroup>();
		if (tGroup) {
			CEntity * eNoiseEmitter = tGroup->getHandleByName("Noise Emitter");
			eNoiseEmitter->sendMsg(msg);
		}
		return BTNode::ERes::LEAVE;
	}
	else {
			rotateTowardsVec(ppos->getPosition(), rotationSpeedChase, dt);
			VEC3 vp = mypos->getPosition();
			VEC3 vfwd = mypos->getFront();
			vfwd.Normalize();
			vp = vp + chaseSpeed * dt * vfwd;
			mypos->setPosition(vp);
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIMimetic::actionMarkNoiseAsInactive(float dt)
{
	timerWaitingInObservation = 0.f;
	hasHeardArtificialNoise = false;
	hasHeardNaturalNoise = false;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionGoToNoiseSource(float dt)
{
	TCompTransform *mypos = get<TCompTransform>();
	rotateTowardsVec(noiseSource, rotationSpeedNoise, dt);

	VEC3 vp = mypos->getPosition();

	CEntity * ePlayer = getEntityByName(entityToChase);
	TCompTransform * ppos = get<TCompTransform>();
	VEC3 pp = ppos->getPosition();

	if (VEC3::Distance(vp, pp) <= autoChaseDistance && isPlayerInFov()) {
		return BTNode::ERes::LEAVE;
	}

	if (VEC3::Distance(noiseSource, vp) < speed * 1.5f * dt) {
		mypos->setPosition(noiseSource);
		return BTNode::ERes::LEAVE;
	}
	else {
		VEC3 vfwd = mypos->getFront();
		vfwd.Normalize();
		vp = vp + speed * 1.5f * dt *vfwd;
		mypos->setPosition(vp);				//Move towards wpt
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIMimetic::actionWaitInNoiseSource(float dt)
{

	TCompTransform *mypos = get<TCompTransform>();
	VEC3 vp = mypos->getPosition();
	CEntity * ePlayer = getEntityByName(entityToChase);
	TCompTransform * ppos = get<TCompTransform>();
	VEC3 pp = ppos->getPosition();

	if (VEC3::Distance(vp, pp) <= autoChaseDistance && isPlayerInFov()) {
		return BTNode::ERes::LEAVE;
	}

	timerWaitingInObservation += dt;
	if (timerWaitingInObservation > 1.f) {
		return BTNode::ERes::LEAVE;
	}
	else {
		return BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIMimetic::actionGoToPlayerLastPos(float dt)
{
	TCompTransform *mypos = get<TCompTransform>();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();
	rotateTowardsVec(lastPlayerKnownPos, rotationSpeedChaseDeg, dt);

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
	if (timerWaitingInWpt < waitTimeInLasPlayerPos) {
		return BTNode::STAY;
	}
	else {
		return BTNode::ERes::LEAVE;
	}
}

BTNode::ERes TCompAIMimetic::actionSetGoInactive(float dt)
{
	TCompRender * cRender = get<TCompRender>();
	cRender->color = VEC4(1, 1, 1, 1);
	goingInactive = true;
	suspectO_Meter = 0.f;
	lastPlayerKnownPos = VEC3::Zero;
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionGoToInitialPos(float dt)
{
	TCompTransform *mypos = get<TCompTransform>();
	rotateTowardsVec(initialPos, rotationSpeedObservation, dt);

	VEC3 vp = mypos->getPosition();

	VEC3 initialPosWithoutY = VEC3(initialPos.x, 0, initialPos.z);
	VEC3 vpWithoutY = VEC3(vp.x, 0, vp.z);


	if (VEC3::Distance(initialPosWithoutY, vpWithoutY) < speed * dt) {
		mypos->setPosition(VEC3(initialPos.x, vp.y, initialPos.z));
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

BTNode::ERes TCompAIMimetic::actionRotateToInitialPos(float dt)
{
	TCompTransform *myPos = get<TCompTransform>();
	bool isInObjective = rotateTowardsVec(myPos->getPosition() + initialLookAt, rotationSpeedObservation, dt);
	return isInObjective ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIMimetic::actionJumpWall(float dt)
{
	TCompRigidbody *tCollider = get<TCompRigidbody>();
	tCollider->Jump(VEC3(0,25.f,0));
	return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionHoldOnWall(float dt)
{

	TCompTransform * tTransform = get<TCompTransform>();
	if (tTransform->getPosition().y >= initialPos.y) {
		tTransform->setPosition(initialPos);
		setGravityToFaceWall();
		return BTNode::ERes::LEAVE;
	}
	return BTNode::ERes::STAY;
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

bool TCompAIMimetic::conditionIsNotPlayerInFovAndNotNoise(float dt)
{
	return !hasHeardNaturalNoise && !hasHeardArtificialNoise && !isPlayerInFov();
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
	return hasHeardNaturalNoise || hasHeardArtificialNoise;
}

bool TCompAIMimetic::conditionNotSurePlayerInFov(float dt)
{
	return suspectO_Meter < 1.f && (suspectO_Meter > 0.f || isPlayerInFov());
}

bool TCompAIMimetic::conditionHasHeardNaturalNoise(float dt)
{
	return hasHeardNaturalNoise;
}

bool TCompAIMimetic::conditionPlayerSeenForSure(float dt)
{
	return isPlayerInFov() && suspectO_Meter >= 1.f;
}

bool TCompAIMimetic::conditionHasHeardArtificialNoise(float dt)
{
	return hasHeardArtificialNoise;
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

bool TCompAIMimetic::assertNotPlayerInFovNorNoise(float dt)
{
	return !hasHeardArtificialNoise && !hasHeardNaturalNoise && !isPlayerInFov();
}

bool TCompAIMimetic::assertNotPlayerInFov(float dt)
{
	return !isPlayerInFov();
}

/* AUX FUNCTIONS */
bool TCompAIMimetic::rotateTowardsVec(VEC3 objective, float rotationSpeed, float dt){
	bool isInObjective = false;
	TCompTransform *mypos = get<TCompTransform>();
	float y, r, p;
	mypos->getYawPitchRoll(&y, &p, &r);
	float deltaYaw = mypos->getDeltaYawToAimTo(objective);
	if (fabsf(deltaYaw) <= rotationSpeed * dt) {
		y += deltaYaw;
		isInObjective = true;
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
	return isInObjective;
}

bool TCompAIMimetic::isPlayerInFov() {

	TCompTransform *mypos = get<TCompTransform>();
	CHandle hPlayer = getEntityByName(entityToChase);
	if (hPlayer.isValid()) {
		CEntity *ePlayer = hPlayer;
		TCompTransform *ppos = ePlayer->get<TCompTransform>();

		float dist = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
		TCompTempPlayerController *pController = ePlayer->get<TCompTempPlayerController>();
		
		/* Player inside cone of vision */
		bool in_fov = mypos->isInFov(ppos->getPosition(), fov, deg2rad(89.f));

		return in_fov && !pController->isMerged && !pController->isDead() && dist <= maxChaseDistance && !isEntityHidden(hPlayer);
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

	CPhysicsCapsule * capsuleCollider = (CPhysicsCapsule *)myCollider->config;

	bool isHidden = true;

	VEC3 myPosition = mypos->getPosition();
	VEC3 origin = myPosition + VEC3(0, capsuleCollider->height * 2, 0);
	VEC3 dest = VEC3::Zero;
	VEC3 dir = VEC3::Zero;

	float i = 0;
	while (isHidden && i < capsuleCollider->height * 2.f) {
		dest = eTransform->getPosition() + VEC3(0.f, Clamp(i - .1f, 0.5f, capsuleCollider->height * 2.f), 0.f);
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

void TCompAIMimetic::setGravityToFaceWall()
{
	TCompRigidbody * tCollider = get<TCompRigidbody>();
	TCompTransform * tTransform = get<TCompTransform>();
	physx::PxRaycastHit hit;

	VEC3 directions[4] = {VEC3(1,0,0), VEC3(-1,0,0), VEC3(0,0,1), VEC3(0,0,-1)};
	float minDistance = INFINITY;
	VEC3 finalDir;

	for (int i = 0; i < 4; i++) {
		

		if (EnginePhysics.Raycast(tTransform->getPosition(), directions[i], 2.f, hit, physx::PxQueryFlag::eSTATIC)) {
			if (hit.distance < minDistance) {
				minDistance = hit.distance;
				finalDir = directions[i];
			}
		}
	}

	tCollider->setNormalGravity(finalDir * 9.8f);
}

TCompAIMimetic::EType TCompAIMimetic::parseStringMimeticType(const std::string & typeString)
{
	if (typeString.compare("floor") == 0) {
		return EType::FLOOR;
	}
	else if (typeString.compare("wall") == 0) {
		return EType::WALL;
	}
	else {
		fatal("Mimetic type not recognized");
		return EType::NUM_TYPES;
	}
	
}

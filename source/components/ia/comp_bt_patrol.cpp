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
	
	if (current) {
		validState = current->getName();
	}
	
	ImGui::Text("Current state: %s", validState.c_str());

	ImGui::Text("Suspect Level:");
	ImGui::SameLine();
	ImGui::ProgressBar(suspectO_Meter);
}

void TCompAIPatrol::load(const json& j, TEntityParseContext& ctx) {
	createRoot("patrol", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
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
	addChild("manageDoPatrol", "managePatrolWasSeen", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionPatrolWasSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "goToWpt", BTNode::EType::ACTION, (BTCondition)&TCompAIPatrol::conditionGoToWpt, (BTAction)&TCompAIPatrol::actionGoToWpt, (BTAssert)&TCompAIPatrol::assertGoToWpt);
	addChild("manageDoPatrol", "waitInWpt", BTNode::EType::ACTION, (BTCondition)&TCompAIPatrol::conditionWaitInWpt, (BTAction)&TCompAIPatrol::actionWaitInWpt, (BTAssert)&TCompAIPatrol::assertWaitInWpt);
	addChild("manageDoPatrol", "nextWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionNextWpt, nullptr);

	addChild("managePlayerSeen", "manageChase", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionChase, nullptr, nullptr);
	addChild("managePlayerSeen", "suspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionSuspect, nullptr);
	addChild("manageChase", "shootInhibitor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionShootInhibitor, nullptr);
	addChild("manageChase", "beginAlertChase", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionBeginAlert, nullptr);
	addChild("manageChase", "manageGoingToAttack", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("manageGoingToAttack", "managePlayerAttacked", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionPlayerAttacked, nullptr, nullptr);
	addChild("manageGoingToAttack", "ChasePlayer", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionChasePlayer, nullptr);
	addChild("managePlayerAttacked", "attack", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionAttack, nullptr);
	addChild("managePlayerAttacked", "endAlertAttacked", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionEndAlert, nullptr);
	addChild("managePlayerAttacked", "closestWptAttacked", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);

	addChild("managePlayerWasSeen", "goToPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionGoToPlayerLastPos, (BTAssert)&TCompAIPatrol::assertPlayerNotInFov);
	addChild("managePlayerWasSeen", "lookForPlayerSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionLookForPlayer, (BTAssert)&TCompAIPatrol::assertPlayerNotInFov);
	addChild("managePlayerWasSeen", "closestWptPlayerSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);

	addChild("managePatrolSeen", "markPatrolAsSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionMarkPatrolAsSeen, nullptr);
	addChild("managePatrolSeen", "goToPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionGoToPatrol, (BTAssert)&TCompAIPatrol::assertPlayerNotInFovAndPatrolInFov);
	addChild("managePatrolSeen", "fixPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionFixPatrol, nullptr);
	addChild("managePatrolSeen", "beginAlertPatrolSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionBeginAlert, nullptr);
	addChild("managePatrolSeen", "closestWptPatrolSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);

	addChild("managePatrolWasSeen", "goToPatrolPrevPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionGoToPatrolPrevPos, (BTAssert)&TCompAIPatrol::assertPlayerAndPatrolNotInFov);
	addChild("managePatrolWasSeen", "lookForPlayerPatrolWasSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionLookForPlayer, (BTAssert)&TCompAIPatrol::assertPlayerAndPatrolNotInFov);
	addChild("managePatrolWasSeen", "closestWptPatrolWasSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);

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
}

void TCompAIPatrol::registerMsgs()
{
	DECL_MSG(TCompAIPatrol, TMsgEntityCreated, onMsgEntityCreated);
}

/* ACTIONS */

BTNode::ERes TCompAIPatrol::actionShadowMerged(float dt)
{
	return BTNode::ERes();
}

BTNode::ERes TCompAIPatrol::actionStunned(float dt)
{
	return BTNode::ERes();
}

BTNode::ERes TCompAIPatrol::actionFixed(float dt)
{
	return BTNode::ERes();
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
		suspectO_Meter += dt * incrBaseSuspectO_Meter;							//TODO: increment more depending distance and noise
		rotateTowardsVec(ppos->getPosition(), dt);
	}
	else {
		suspectO_Meter -= dt * dcrSuspectO_Meter;
	}

	if (suspectO_Meter <= 0.f || suspectO_Meter >= 1.f) {
		if (suspectO_Meter <= 0) {
			cRender->color = VEC4(255, 0, 0, 1);
		}
		return BTNode::ERes::LEAVE;
	}
	else {
		return BTNode::ERes::STAY;
	}
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

BTNode::ERes TCompAIPatrol::actionGoToPlayerLastPos(float dt)
{
	return BTNode::ERes();
}

BTNode::ERes TCompAIPatrol::actionLookForPlayer(float dt)
{
	return BTNode::ERes();
}

BTNode::ERes TCompAIPatrol::actionMarkPatrolAsSeen(float dt)
{
	return BTNode::ERes();
}

BTNode::ERes TCompAIPatrol::actionGoToPatrol(float dt)
{
	return BTNode::ERes();
}

BTNode::ERes TCompAIPatrol::actionFixPatrol(float dt)
{
	return BTNode::ERes();
}

BTNode::ERes TCompAIPatrol::actionGoToPatrolPrevPos(float dt)
{
	return BTNode::ERes();
}


/* CONDITIONS */

bool TCompAIPatrol::conditionManageStun(float dt)
{
	return false;
}

bool TCompAIPatrol::conditionEndAlert(float dt)
{
	return false;
}

bool TCompAIPatrol::conditionShadowMerged(float dt)
{
	return false;
}

bool TCompAIPatrol::conditionFixed(float dt)
{
	return false;
}

bool TCompAIPatrol::conditionPlayerSeen(float dt)
{
	return isPlayerInFov();
}

bool TCompAIPatrol::conditionPlayerWasSeen(float dt)
{
	return false;
}

bool TCompAIPatrol::conditionPatrolSeen(float dt)
{
	return false;
}

bool TCompAIPatrol::conditionPatrolWasSeen(float dt)
{
	return false;
}

bool TCompAIPatrol::conditionGoToWpt(float dt)
{
	TCompTransform* mypos = get<TCompTransform>();
	return VEC3::Distance(mypos->getPosition(), _waypoints[currentWaypoint].position) != 0;
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

bool TCompAIPatrol::assertGoToWpt(float dt)
{
	return !isPlayerInFov();
}

bool TCompAIPatrol::assertWaitInWpt(float dt)
{
	return !isPlayerInFov() /* && !isPatrolInFov() */;
}

bool TCompAIPatrol::assertPlayerInFov(float dt)
{
	return false;
}

bool TCompAIPatrol::assertPlayerNotInFov(float dt)
{
	return false;
}

bool TCompAIPatrol::assertPlayerNotInFovAndPatrolInFov(float dt)
{
	return false;
}

bool TCompAIPatrol::assertPlayerAndPatrolNotInFov(float dt)
{
	return false;
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
		TCompPlayerController *pController = ePlayer->get<TCompPlayerController>();
		
		/* Player inside cone of vision */
		bool in_fov = mypos->isInFov(ppos->getPosition(), fov);

		return in_fov && !pController->isInShadows() && !pController->isDead() && !isEntityHidden(hPlayer);
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

#include "mcv_platform.h"
#include "handle/handle.h"
#include "ai_patrol.h"
#include "entity/entity_parser.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_controller.h"
#include "components/comp_render.h"
#include "render/render_utils.h"
#include "components/comp_group.h"
#include "components/object_controller/comp_cone_of_light.h"

DECL_OBJ_MANAGER("ai_patrol", CAIPatrol);

void CAIPatrol::Init()
{
	// insert all states in the map
	AddState("idle", (statehandler)&CAIPatrol::IdleState);
	AddState("goToWpt", (statehandler)&CAIPatrol::GoToWptState);
	AddState("waitInWpt", (statehandler)&CAIPatrol::WaitInWptState);
	AddState("nextWpt", (statehandler)&CAIPatrol::NextWptState);
	AddState("closestWpt", (statehandler)&CAIPatrol::ClosestWptState);
	AddState("suspect", (statehandler)&CAIPatrol::SuspectState);
	AddState("shootInhibitor", (statehandler)&CAIPatrol::ShootInhibitorState);
	AddState("chase", (statehandler)&CAIPatrol::ChaseState);
	AddState("attack", (statehandler)&CAIPatrol::AttackState);
	AddState("idleWar", (statehandler)&CAIPatrol::IdleWarState);
	AddState("goToNoise", (statehandler)&CAIPatrol::GoToNoiseState);
	AddState("goToPatrol", (statehandler)&CAIPatrol::GoToPatrolState);
	AddState("fixPatrol", (statehandler)&CAIPatrol::FixOtherPatrolState);
	AddState("goToPlayerLastPos", (statehandler)&CAIPatrol::GoPlayerLastPosState);
	AddState("seekPlayer", (statehandler)&CAIPatrol::SeekPlayerState);
	AddState("stunned", (statehandler)&CAIPatrol::StunnedState);
	AddState("fixed", (statehandler)&CAIPatrol::FixedState);
	AddState("shadowMerged", (statehandler)&CAIPatrol::ShadowMergedState);

	// reset the state
	ChangeState("idle");
	currentWaypoint = 0;
}

void CAIPatrol::debugInMenu() {
	IAIController::debugInMenu();
	if (ImGui::TreeNode("Waypoints")) {
		for (auto& v : _waypoints) {
			ImGui::PushID(&v);
			ImGui::DragFloat3("Point", &v.position.x, 0.1f, -20.f, 20.f);
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
	ImGui::Text("Suspect Level:");
	ImGui::SameLine();
	ImGui::ProgressBar(suspectO_Meter);

	if (_waypoints.size() > 1) {
		for (size_t i = 0; i < _waypoints.size(); ++i) {
			renderLine(_waypoints[i].position, _waypoints[(i + 1) % _waypoints.size()].position, VEC4(0, 1, 0, 1));
		}
	}

	if (lastPlayerKnownPos != VEC3::Zero) {
		renderLine(((TCompTransform *)get<TCompTransform>())->getPosition(), lastPlayerKnownPos, VEC4(255, 0, 0, 1));
	}
}

void CAIPatrol::load(const json& j, TEntityParseContext& ctx) {
	setEntity(ctx.current_entity);

	Init();

	auto& j_waypoints = j["waypoints"];
	for (auto it = j_waypoints.begin(); it != j_waypoints.end(); ++it) {

		Waypoint wpt;
		assert(it.value().count("position") == 1);
		assert(it.value().count("lookAt") == 1);

		wpt.position = loadVEC3(it.value()["position"]);
		wpt.lookAt = loadVEC3(it.value()["lookAt"]);
		wpt.minTime = it.value().value("minTime", 5.f);
		addWaypoint(wpt);
	}

	speed = j.value("speed", 2.0f);
	rotationSpeed = deg2rad(j.value("rotationSpeed", 90));
	fov = deg2rad(j.value("fov", 60));
	entityToChase = j.value("entityToChase", "The Player");
	autoChaseDistance = j.value("autoChaseDistance", 5.f);
	maxChaseDistance = j.value("maxChaseDistance", 10.f);
	maxTimeSuspecting = j.value("maxTimeSuspecting", 3.f);
	dcrSuspectO_Meter = j.value("dcrSuspectO_meter", .3f);
	incrBaseSuspectO_Meter = j.value("incrBaseSuspectO_meter", .3f);
	distToAttack = j.value("distToIdleWar", 1.0f);
	maxRotationSeekingPlayer = deg2rad(j.value("maxRotationSeekingPlayer", 90));

	//  distToBack = j.value("distToBack", 1.0f);
	//  distToChase = j.value("distToChase", 10.0f);
	//  entityToChase = j.value("entityToChase", "The Player");
	//  life = j.value("life", 5);
	//  idleWarTimerBase = j.value("idleWarTimerBase", 1.0f);
	//  idleWarTimerExtra = j.value("idleWarTimerExtra", 2);
	//  orbitRotationBase = deg2rad(j.value("orbitRotationBase", 60));
	//  orbitRotationExtra = j.value("orbitRotationExtra", 30);
}

void CAIPatrol::registerMsgs() {									//TODO: Change
	DECL_MSG(CAIPatrol, TMsgPlayerDead, onMsgPlayerDead);					//TODO: Change
	DECL_MSG(CAIPatrol, TMsgPatrolStunned, onMsgPatrolStunned);					//TODO: Change
	DECL_MSG(CAIPatrol, TMsgPatrolShadowMerged, onMsgPatrolShadowMerged);
}																	//TODO: Change

void CAIPatrol::onMsgPlayerDead(const TMsgPlayerDead& msg) {
	suspectO_Meter = 0.f;
	lastPlayerKnownPos = VEC3::Zero;
	TCompRender *cRender = get<TCompRender>();
	cRender->color = VEC4(1, 1, 1, 1);
	ChangeState("closestWpt");
}

void CAIPatrol::onMsgPatrolStunned(const TMsgPatrolStunned& msg) {
	TCompRender *cRender = get<TCompRender>();
	cRender->color = VEC4(1, 1, 1, 1);
	TCompTransform *mypos = getMyTransform();
	float y, p, r;
	mypos->getYawPitchRoll(&y, &p, &r);
	p = p + deg2rad(90.f);
	mypos->setYawPitchRoll(y, p, r);
	ChangeState("stunned");
}

void CAIPatrol::onMsgPatrolShadowMerged(const TMsgPatrolShadowMerged& msg) {
	TCompRender *cRender = get<TCompRender>();
	cRender->color = VEC4(0, 0, 0, 0);
	ChangeState("shadowMerged");
}

void CAIPatrol::IdleState(float dt)
{
	ChangeState("goToWpt");
}

/**
Moves to currentWpt position
*/
void CAIPatrol::GoToWptState(float dt)
{
	TCompTransform *mypos = getMyTransform();
	rotateTowardsVec(getWaypoint().position, dt);

	VEC3 vp = mypos->getPosition();

	if (VEC3::Distance(getWaypoint().position, vp) < speed * dt) {
		mypos->setPosition(getWaypoint().position);
		ChangeState("waitInWpt");
	}
	else {
		VEC3 vfwd = mypos->getFront();
		vfwd.Normalize();
		vp = vp + speed * dt *vfwd;
		mypos->setPosition(vp);				//Move towards wpt
	}

	if (isPlayerInFov()) {
		TCompTransform *mypos = getMyTransform();
		CEntity *player = (CEntity *)getEntityByName(entityToChase);
		TCompTransform *ppos = player->get<TCompTransform>();

		/* Distance to player */
		float distanceToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
		if (distanceToPlayer < maxChaseDistance) {
			TCompRender *cRender = get<TCompRender>();
			cRender->color = VEC4(255, 255, 0, 1);
			ChangeState("suspect");
		}
	}

	//// next wpt
	//if (VEC3::Distance(getWaypoint(), vp) < 1) ChangeState("nextWpt");

	//// chase
	//CEntity *player = (CEntity *)getEntityByName(entityToChase);
	//TCompTransform *ppos = player->get<TCompTransform>();
	//bool in_fov = mypos->isInFov(ppos->getPosition(), fov);
	//if (in_fov && VEC3::Distance(mypos->getPosition(), ppos->getPosition()) <= distToChase) {
	// ChangeState("chase");
	//}
}

/**
Waits in the current position the time set in the currentWpt unless
it sees the enemy
*/
void CAIPatrol::WaitInWptState(float dt)
{
	if (timerWaitingInWpt >= getWaypoint().minTime) {
		timerWaitingInWpt = 0.f;
		ChangeState("nextWpt");
	}
	else {
		timerWaitingInWpt += dt;
		TCompTransform *mypos = getMyTransform();
		rotateTowardsVec(mypos->getPosition() + getWaypoint().lookAt, dt);
	}

	if (isPlayerInFov()) {
		TCompTransform *mypos = getMyTransform();
		CEntity *player = (CEntity *)getEntityByName(entityToChase);
		TCompTransform *ppos = player->get<TCompTransform>();

		/* Distance to player */
		float distanceToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
		if (distanceToPlayer < maxChaseDistance) {
			TCompRender * cRender = get<TCompRender>();
			cRender->color = VEC4(255, 255, 0, 1);
			ChangeState("suspect");
		}
	}
}

void CAIPatrol::NextWptState(float dt)
{
	currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
	ChangeState("goToWpt");
}


void CAIPatrol::ClosestWptState(float dt) {
	float minDistance = INFINITY;
	int  minIndexWpt = 0;

	for (int i = 0; i < _waypoints.size(); i++) {
		float currDistance = VEC3::Distance(getMyTransform()->getPosition(), _waypoints[i].position);
		if (currDistance < minDistance) {
			minDistance = currDistance;
			minIndexWpt = i;
		}
	}

	currentWaypoint = minIndexWpt;
	ChangeState("goToWpt");
}

/**
The enemy thinks he may have seen the player. Rotates towards him and wait until he is sure
(this will depend on the distance and player's noise)
*/
void CAIPatrol::SuspectState(float dt)
{
	// chase
	TCompTransform *mypos = getMyTransform();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	/* Distance to player */
	float distanceToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());

	if (isPlayerInFov() && distanceToPlayer <= autoChaseDistance) {
		suspectO_Meter = 1.f;
		rotateTowardsVec(ppos->getPosition(), dt);
	}
	else if (isPlayerInFov() && distanceToPlayer <= maxChaseDistance) {
		suspectO_Meter += dt * incrBaseSuspectO_Meter;							//TODO: increment more depending distance and noise
		rotateTowardsVec(ppos->getPosition(), dt);
	}
	else {
		suspectO_Meter -= dt * dcrSuspectO_Meter;
	}

	if (suspectO_Meter >= 1.f) {
		TCompRender *cRender = get<TCompRender>();
		cRender->color = VEC4(255, 0, 0, 1);
		ChangeState("shootInhibitor");
	}
	else if (suspectO_Meter <= 0.f) {
		if (patrolDeadPosition != VEC3::Zero) {
			//TODO: revive patrol
		}
		else {
			TCompRender *cRender = get<TCompRender>();
			cRender->color = VEC4(1, 1, 1, 1);
			ChangeState("goToWpt");
		}
	}
}

void CAIPatrol::ShootInhibitorState(float dt)
{
	turnOnLight();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TMsgInhibitorShot msg;
	msg.h_sender = CHandle(this).getOwner();
	player->sendMsg(msg);
	ChangeState("chase");
}


void CAIPatrol::ChaseState(float dt)
{
	TCompTransform *mypos = getMyTransform();
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
		ChangeState("goToPlayerLastPos");
	}
	else if (distToPlayer < distToAttack) {
		ChangeState("attack");
	}
	else {
		rotateTowardsVec(ppos->getPosition(), dt);
		VEC3 vp = mypos->getPosition();
		VEC3 vfwd = mypos->getFront();
		vfwd.Normalize();
		vp = vp + speed * dt * vfwd;
		mypos->setPosition(vp);
	}
}

void CAIPatrol::AttackState(float dt)
{

	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform * ppos = player->get<TCompTransform>();

	if (lastPlayerKnownPos != VEC3::Zero) {

		/* If we had the player's previous position, know where he is going */
		isLastPlayerKnownDirLeft = getMyTransform()->isInLeft(ppos->getPosition() - lastPlayerKnownPos);
	}
	lastPlayerKnownPos = ppos->getPosition();

	if (false) {				//TODO: see if the player is hit => is dead
		ChangeState("idleWar");
	}
	else {
		// Notify the entity that he is dead
		TMsgPlayerHit msg;
		msg.h_sender = CHandle(this).getOwner();      // Who killed the player
		CEntity *player = (CEntity *)getEntityByName(entityToChase);
		player->sendMsg(msg);
	}
}


void CAIPatrol::IdleWarState(float dt) {
	/*TCompTransform *mypos = getMyTransform();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	if (VEC3::Distance(mypos->getPosition(), ppos->getPosition()) <= distToBack) {
	idleWarTimer = 0;
	ChangeState("back");
	}
	else if(VEC3::Distance(mypos->getPosition(), ppos->getPosition()) > distToIdleWar + 0.5){
	idleWarTimer = 0;
	ChangeState("chase");
	}
	else {

	float y, r, p;
	mypos->getYawPitchRoll(&y, &p, &r);
	if (mypos->isInLeft(ppos->getPosition()))
	{
	y += rotationSpeed * dt;
	}
	else
	{
	y -= rotationSpeed * dt;
	}
	mypos->setYawPitchRoll(y, p, r);

	if (idleWarTimer >= idleWarTimerMax) {
	idleWarTimer = 0;
	ChangeState("chooseOrbitSide");
	}
	else {
	idleWarTimer += dt;
	}
	}*/
}

void CAIPatrol::GoToNoiseState(float dt)
{
}

void CAIPatrol::GoToPatrolState(float dt)
{

}

void CAIPatrol::FixOtherPatrolState(float dt)
{

}

void CAIPatrol::GoPlayerLastPosState(float dt)
{
	TCompTransform *mypos = getMyTransform();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();
	rotateTowardsVec(lastPlayerKnownPos, dt);

	VEC3 vp = mypos->getPosition();

	if (isPlayerInFov() && VEC3::Distance(mypos->getPosition(), ppos->getPosition()) < maxChaseDistance) {
		TCompRender * cRender = get<TCompRender>();
		cRender->color = VEC4(255, 255, 0, 1);
		ChangeState("suspect");
	}
	else {
		if (VEC3::Distance(lastPlayerKnownPos, vp) < speed * dt) {
			mypos->setPosition(lastPlayerKnownPos);
			lastPlayerKnownPos = VEC3::Zero;
			ChangeState("seekPlayer");
		}
		else {
			VEC3 vfwd = mypos->getFront();
			vfwd.Normalize();
			vp = vp + speed * dt * vfwd;
			mypos->setPosition(vp);

			//TODO: see if its better to change state here, or in the msg we are going to receive
		}
	}
}

void CAIPatrol::SeekPlayerState(float dt)
{

	TCompTransform *mypos = getMyTransform();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	if (isPlayerInFov() && VEC3::Distance(mypos->getPosition(), ppos->getPosition()) < maxChaseDistance) {
		TCompRender * cRender = get<TCompRender>();
		cRender->color = VEC4(255, 255, 0, 1);
		amountRotated = 0.f;
		ChangeState("suspect");
	}
	else {

		if (amountRotated >= maxRotationSeekingPlayer * 3) {
			suspectO_Meter = 0.f;
			TCompRender * cRender = get<TCompRender>();
			cRender->color = VEC4(1, 1, 1, 1);
			amountRotated = 0.f;
			ChangeState("closestWpt");
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
		}
	}
}

void CAIPatrol::StunnedState(float dt)
{

}

void CAIPatrol::FixedState(float dt)
{
}

void CAIPatrol::ShadowMergedState(float dt)
{
	/* Destroy the entity */
	CHandle(this).getOwner().destroy();
}

void CAIPatrol::BackState(float dt) {
	/*TCompTransform *mypos = getMyTransform();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();
	float y, r, p;
	mypos->getYawPitchRoll(&y, &p, &r);
	if (mypos->isInLeft(ppos->getPosition()))
	{
	y += rotationSpeed * dt;
	}
	else
	{
	y -= rotationSpeed * dt;
	}
	mypos->setYawPitchRoll(y, p, r);
	VEC3 vp = mypos->getPosition();
	VEC3 vfwd = mypos->getFront();
	vfwd.Normalize();
	vp = vp - speed * dt * vfwd;
	mypos->setPosition(vp);

	if (VEC3::Distance(mypos->getPosition(), ppos->getPosition()) > distToBack + 0.5f) {
	idleWarTimerMax = idleWarTimerBase + (rand() % idleWarTimerExtra);
	ChangeState("idleWar");
	}*/
}

void CAIPatrol::rotateTowardsVec(VEC3 objective, float dt) {
	TCompTransform *mypos = getMyTransform();
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

bool CAIPatrol::isPlayerInFov() {
	// chase
	TCompTransform *mypos = getMyTransform();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();
	TCompPlayerController *pController = player->get <TCompPlayerController>();

	/* Player inside cone of vision */
	bool in_fov = mypos->isInFov(ppos->getPosition(), fov);

	return in_fov && !pController->isInShadows() && !pController->isDead();
}



void CAIPatrol::turnOnLight()
{
	TCompGroup* cGroup = get<TCompGroup>();
	CEntity* eCone = cGroup->getHandleByName("Cone of Light");
	TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
	cConeController->turnOnLight();
}

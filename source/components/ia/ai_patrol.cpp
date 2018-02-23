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
	ImGui::Text("Speed %f", speed);

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

	speed = j.value("speed", speed);
	rotationSpeedDeg = j.value("rotationSpeed", rotationSpeedDeg);
	fovDeg = j.value("fov", fovDeg);
	entityToChase = j.value("entityToChase", entityToChase);
	autoChaseDistance = j.value("autoChaseDistance", autoChaseDistance);
	maxChaseDistance = j.value("maxChaseDistance", maxChaseDistance);
	maxTimeSuspecting = j.value("maxTimeSuspecting", maxTimeSuspecting);
	dcrSuspectO_Meter = j.value("dcrSuspectO_meter", dcrSuspectO_Meter);
	incrBaseSuspectO_Meter = j.value("incrBaseSuspectO_meter", incrBaseSuspectO_Meter);
	distToAttack = j.value("distToIdleWar", distToAttack);
	maxRotationSeekingPlayerDeg = j.value("maxRotationSeekingPlayer", maxRotationSeekingPlayerDeg);

	rotationSpeed = deg2rad(rotationSpeedDeg);
	fov = deg2rad(fovDeg);
	maxRotationSeekingPlayer = deg2rad(maxRotationSeekingPlayerDeg);

	//  distToBack = j.value("distToBack", 1.0f);
	//  distToChase = j.value("distToChase", 10.0f);
	//  entityToChase = j.value("entityToChase", "The Player");
	//  life = j.value("life", 5);
	//  idleWarTimerBase = j.value("idleWarTimerBase", 1.0f);
	//  idleWarTimerExtra = j.value("idleWarTimerExtra", 2);
	//  orbitRotationBase = deg2rad(j.value("orbitRotationBase", 60));
	//  orbitRotationExtra = j.value("orbitRotationExtra", 30);
}

void CAIPatrol::registerMsgs() {
	DECL_MSG(CAIPatrol, TMsgEntityCreated, onMsgPatrolCreated);
	DECL_MSG(CAIPatrol, TMsgPlayerDead, onMsgPlayerDead);
	DECL_MSG(CAIPatrol, TMsgPatrolStunned, onMsgPatrolStunned);
	DECL_MSG(CAIPatrol, TMsgPatrolShadowMerged, onMsgPatrolShadowMerged);
	DECL_MSG(CAIPatrol, TMsgPatrolFixed, onMsgPatrolFixed);
}

void CAIPatrol::onMsgPatrolCreated(const TMsgEntityCreated& msg) {

	if(_waypoints.size() == 0){
		Waypoint wpt;
		wpt.position = ((TCompTransform*)get<TCompTransform>())->getPosition();
		wpt.lookAt = ((TCompTransform*)get<TCompTransform>())->getFront();
		wpt.minTime = 1.f;
		addWaypoint(wpt);
	}
}

void CAIPatrol::onMsgPlayerDead(const TMsgPlayerDead& msg) {
	if (getStateName().compare("stunned") != 0) {
		suspectO_Meter = 0.f;
		lastPlayerKnownPos = VEC3::Zero;
		TCompRender *cRender = get<TCompRender>();
		cRender->color = VEC4(1, 1, 1, 1);
		turnOffLight();
		ChangeState("closestWpt");
	}
}

void CAIPatrol::onMsgPatrolStunned(const TMsgPatrolStunned& msg) {
	TCompRender *cRender = get<TCompRender>();
	cRender->color = VEC4(1, 1, 1, 1);
	TCompTransform *mypos = getMyTransform();
	float y, p, r;
	mypos->getYawPitchRoll(&y, &p, &r);
	p = p + deg2rad(89.9f);
	mypos->setYawPitchRoll(y, p, r);
	turnOffLight();

	TCompGroup* cGroup = get<TCompGroup>();
	CEntity* eCone = cGroup->getHandleByName("Cone of Vision");
	TCompRender * coneRender = eCone->get<TCompRender>();
	coneRender->visible = false;

	lastPlayerKnownPos = VEC3::Zero;

	/* Tell the other patrols I am stunned */
	CEngine::get().getIA().patrolSB.stunnedPatrols.emplace_back(CHandle(this).getOwner());

	ChangeState("stunned");
}

void CAIPatrol::onMsgPatrolShadowMerged(const TMsgPatrolShadowMerged& msg) {
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

	ChangeState("shadowMerged");
}

void CAIPatrol::onMsgPatrolFixed(const TMsgPatrolFixed& msg) {

	if (getStateName().compare("stunned") == 0) {
		TCompTransform *mypos = getMyTransform();
		float y, p, r;
		mypos->getYawPitchRoll(&y, &p, &r);
		p = p - deg2rad(89.9f);
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

		ChangeState("idle");
	}
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
	VEC3 patata = vp;
	dbg("\nPatrol: LastFrame: %f %f %f", vp.x, vp.y, vp.z );
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
	VEC3 delta = vp - patata;
	dbg("      newPosition: %f %f %f\n",vp.x, vp.y, vp.z);
	dbg("\nPatrol Delta Movement %f %f %f\n", delta.x, delta.y, delta.z);
	dbg("");
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
	else if (isStunnedPatrolInFov() || lastStunnedPatrolKnownPos != VEC3::Zero) {
		ChangeState("goToPatrol");
	}
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
	else if (isStunnedPatrolInFov() || lastStunnedPatrolKnownPos != VEC3::Zero) {
		ChangeState("goToPatrol");
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
		if (isStunnedPatrolInFov() || lastStunnedPatrolKnownPos != VEC3::Zero) {
			ChangeState("goToPatrol");
		}
		else {
			TCompRender *cRender = get<TCompRender>();
			cRender->color = VEC4(1, 1, 1, 1);
			ChangeState("goToWpt");
		}
	}

	if (lastStunnedPatrolKnownPos == VEC3::Zero) {
		isStunnedPatrolInFov();
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

	if (lastStunnedPatrolKnownPos == VEC3::Zero) {
		isStunnedPatrolInFov();
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

}

void CAIPatrol::GoToNoiseState(float dt)
{
}

void CAIPatrol::GoToPatrolState(float dt)
{
	TCompTransform *mypos = getMyTransform();
	rotateTowardsVec(lastStunnedPatrolKnownPos, dt);

	VEC3 vp = mypos->getPosition();

	if (VEC3::Distance(vp, lastStunnedPatrolKnownPos) < distToAttack) {
		if (isStunnedPatrolInPos(lastStunnedPatrolKnownPos)) {
			ChangeState("fixPatrol");
		}
		else {
			lastStunnedPatrolKnownPos = VEC3::Zero;
			ChangeState("seekPlayer");
		}
		
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
}

void CAIPatrol::FixOtherPatrolState(float dt)
{
	CEntity* eStunnedPatrol = getPatrolInPos(lastStunnedPatrolKnownPos);
	TMsgPatrolFixed msg;
	msg.h_sender = CHandle(this).getOwner();
	eStunnedPatrol->sendMsg(msg);
	lastStunnedPatrolKnownPos = VEC3::Zero;
	turnOnLight();
	ChangeState("closestWpt");
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
	if (lastStunnedPatrolKnownPos == VEC3::Zero) {
		isStunnedPatrolInFov();
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
		if (lastStunnedPatrolKnownPos == VEC3::Zero) {
			isStunnedPatrolInFov();
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

	TCompTransform *mypos = getMyTransform();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();
	TCompPlayerController *pController = player->get <TCompPlayerController>();

	/* Player inside cone of vision */
	bool in_fov = mypos->isInFov(ppos->getPosition(), fov);

	return in_fov && !pController->isInShadows() && !pController->isDead();
}

bool CAIPatrol::isStunnedPatrolInFov()
{
	std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;

	bool found = false;

	if (stunnedPatrols.size() > 0) {
		TCompTransform *mypos = getMyTransform();
		for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
			TCompTransform* stunnedPatrol = ((CEntity*)stunnedPatrols[i])->get<TCompTransform>();
			if (mypos->isInFov(stunnedPatrol->getPosition(), fov) 
				&& VEC3::Distance(mypos->getPosition(), stunnedPatrol->getPosition()) < maxChaseDistance) {
				found = true;
				lastStunnedPatrolKnownPos = stunnedPatrol->getPosition();
			}
		}
	}

	return found;
}

bool CAIPatrol::isStunnedPatrolInPos(VEC3 lastPos)
{
	std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;

	bool found = false;

	if (stunnedPatrols.size() > 0) {
		TCompTransform *mypos = getMyTransform();
		for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
			TCompTransform* stunnedPatrol = ((CEntity*)stunnedPatrols[i])->get<TCompTransform>();
			if (stunnedPatrol->getPosition() == lastPos){
				found = true;
			}
		}
	}

	return found;
}

CHandle CAIPatrol::getPatrolInPos(VEC3 lastPos)
{
	std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;

	bool found = false;
	CHandle h_stunnedPatrol;

	if (stunnedPatrols.size() > 0) {
		TCompTransform *mypos = getMyTransform();
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

void CAIPatrol::turnOnLight()
{
	TCompGroup* cGroup = get<TCompGroup>();
	CEntity* eCone = cGroup->getHandleByName("Cone of Light");
	TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
	cConeController->turnOnLight();
}

void CAIPatrol::turnOffLight() {
	TCompGroup* cGroup = get<TCompGroup>();
	CEntity* eCone = cGroup->getHandleByName("Cone of Light");
	TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
	cConeController->turnOffLight();
}

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
}

void CAIPatrol::renderDebug() {
	//// AJUSTE PARÁMETROS

	//CEntity* e = CHandle(this).getOwner();
	//std::string nombreAux = e->getName();
	//std::string nombre = "Enemigo ";
	//nombre = nombre + nombreAux;

	//ImGui::Begin("Ajuste parametros:");
	//if (ImGui::CollapsingHeader(nombre.c_str())) {
	//	ImGui::Text("Vel. enemigo %s", e->getName());
	//	ImGui::DragFloat("", &speed, 0.1f, 0.1f, 20.f);
	//	ImGui::Text("Vel. rot. enemigo %s", e->getName());
	//	ImGui::DragFloat(" ", &rotationSpeed, 0.1f, 0.1f, 20.f);
	//	ImGui::Text("Fov %s", e->getName());
	//	ImGui::DragFloat("  ", &fov, 0.1f, 0.1f, 30.f);
	//	ImGui::Text("Distancia para atacar al player 100x100 %s", e->getName());
	//	ImGui::DragFloat("   ", &autoChaseDistance, 0.1f, 0.1f, 20.f);
	//	ImGui::Text("Distancia max ver player %s", e->getName());
	//	ImGui::DragFloat("    ", &maxChaseDistance, 0.1f, 0.1f, 20.f);
	//	ImGui::Text("Max tiempo sospecha %s", e->getName());
	//	ImGui::DragFloat("      ", &maxTimeSuspecting, 0.5f, 0.5f, 5.f);
	//	ImGui::Text("Decr. sospecha %s", e->getName());
	//	ImGui::DragFloat("         ", &dcrSuspectO_Meter, 0.05f, 0.05f, .5f);
	//	ImGui::Text("Incr. sospecha %s", e->getName());
	//	ImGui::DragFloat("            ", &incrBaseSuspectO_Meter, 0.05f, 0.05f, .5f);
	//	ImGui::Text("Dist para atacar %s", e->getName());
	//	ImGui::DragFloat("               ", &distToAttack, 0.1f, 0.1f, 2.f);
	//}
	//ImGui::End();
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
	distToAttack = j.value("distToAttack", 1.5f);
	maxRotationSeekingPlayerDeg = j.value("maxRotationSeekingPlayer", maxRotationSeekingPlayerDeg);
	startLightsOn = j.value("startLightsOn", startLightsOn);

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

		ChangeState("idle");
	}
}

void CAIPatrol::IdleState(float dt)
{
	if (startLightsOn) {
		turnOnLight();
	}
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

	if (VEC3::Distance(vp, lastStunnedPatrolKnownPos) < distToAttack + 1.f) {
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

	if (VEC3::Distance(mypos->getPosition(), ppos->getPosition()) < maxChaseDistance && isPlayerInFov()) {
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

	if (VEC3::Distance(mypos->getPosition(), ppos->getPosition()) < maxChaseDistance && isPlayerInFov()) {
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

	/* Player inside cone of vision */
	bool in_fov = mypos->isInFov(ppos->getPosition(), fov);

	return in_fov && !isEntityHidden(getEntityByName(entityToChase));
}

bool CAIPatrol::isEntityHidden(CHandle h_entity)
{
	CEntity *entity = h_entity;
	TCompTransform *mypos = getMyTransform();
	TCompTransform *eTransform = entity->get<TCompTransform>();
	TCompCollider *myCollider = get<TCompCollider>();
	TCompCollider *eCollider = entity->get<TCompCollider>();

	bool isHidden = true;

	VEC3 myPosition = mypos->getPosition();
	VEC3 origin = myPosition + VEC3(0, 3 * 2, 0);
	VEC3 dest = VEC3::Zero;
	VEC3 dir = VEC3::Zero;

	float i = 0;
	while (isHidden && i < 3 * 2) {
		dest = eTransform->getPosition() + VEC3(0, Clamp(i - .1f, 0.f, 3.f * 2), 0);
		dir = dest - origin;
		dir.Normalize();
		physx::PxRaycastHit hit;
		float dist = VEC3::Distance(origin,dest);

		//TODO: only works when behind scenery. Make the same for other enemies, dynamic objects...
		if (!EnginePhysics.Raycast(origin, dir, dist, hit, physx::PxQueryFlag::eSTATIC)) {
			isHidden = false;
		}
		i = i + (3 / 2);
	}
	return isHidden;
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
				&& VEC3::Distance(mypos->getPosition(), stunnedPatrol->getPosition()) < maxChaseDistance
				&& !isEntityHidden(stunnedPatrols[i])) {
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

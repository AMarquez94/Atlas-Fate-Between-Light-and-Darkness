#include "mcv_platform.h"
#include "handle/handle.h"
#include "ai_patrol.h"
#include "entity/entity_parser.h"
#include "components/comp_transform.h"
#include "render/render_utils.h"

DECL_OBJ_MANAGER("ai_patrol", CAIPatrol);

void CAIPatrol::Init()
{
  // insert all states in the map
  AddState("idle", (statehandler)&CAIPatrol::IdleState);
  AddState("seekWpt", (statehandler)&CAIPatrol::SeekWptState);
  AddState("waitWpt", (statehandler)&CAIPatrol::WaitInWptState);
  AddState("nextWpt", (statehandler)&CAIPatrol::NextWptState);
  AddState("closestWpt", (statehandler)&CAIPatrol::ClosestWptState);
  AddState("suspect", (statehandler)&CAIPatrol::SuspectState);
  AddState("shootInhibitor", (statehandler)&CAIPatrol::ShootInhibitorState);
  AddState("chase", (statehandler)&CAIPatrol::ChaseState);
  AddState("attack", (statehandler)&CAIPatrol::AttackState);
  AddState("idleWar", (statehandler)&CAIPatrol::IdleWarState);
  AddState("beginAlert", (statehandler)&CAIPatrol::BeginAlertState);
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

  for (size_t i = 0; i < _waypoints.size(); ++i)
    renderLine(_waypoints[i].position, _waypoints[(i + 1) % _waypoints.size()].position, VEC4(0, 1, 0, 1));
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
	  addWaypoint(wpt);
  }

//  speed = j.value("speed", 2.0f);
//  rotationSpeed = deg2rad(j.value("rotationSpeed", 90));
//  fov = deg2rad(j.value("fov", 60));
//  distToIdleWar = j.value("distToIdleWar", 2.0f);
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
	DECL_MSG(CAIPatrol, TMsgDamage, onMsgDamage);
}

void CAIPatrol::onMsgDamage(const TMsgDamage& msg) {
	ChangeState("hit");
}


void CAIPatrol::IdleState(float dt)
{
  ChangeState("seekWpt");
}


void CAIPatrol::SeekWptState(float dt)
{
  //TCompTransform *mypos = getMyTransform();
  //float y, r, p;
  //mypos->getYawPitchRoll(&y, &p, &r);
  //if (mypos->isInLeft(getWaypoint()))
  //{
  //  y += rotationSpeed * dt;
  //}
  //else
  //{
  //  y -= rotationSpeed * dt;
  //}
  //mypos->setYawPitchRoll(y, p, r);
  //VEC3 vp = mypos->getPosition();
  //VEC3 vfwd = mypos->getFront();
  //vfwd.Normalize();
  //vp = vp + speed * dt *vfwd;
  //mypos->setPosition(vp);
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

void CAIPatrol::WaitInWptState(float dt)
{
}

void CAIPatrol::NextWptState(float dt)
{
  currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
  ChangeState("seekWpt");
}


void CAIPatrol::ClosestWptState(float dt) {
	/*float minDistance = INFINITY;
	int  minIndexWpt = 0;

	for (int i = 0; i < _waypoints.size(); i++) {
		float currDistance = VEC3::Distance(getMyTransform()->getPosition(), _waypoints[i]);
		if (currDistance < minDistance) {
			minDistance = currDistance;
			minIndexWpt = i;
		}
	}

	currentWaypoint = minIndexWpt;
	ChangeState("seekWpt");*/
}

void CAIPatrol::SuspectState(float dt)
{
}

void CAIPatrol::ShootInhibitorState(float dt)
{
}


void CAIPatrol::ChaseState(float dt)
{
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
  vp = vp + speed * dt * vfwd;
  mypos->setPosition(vp);

  bool in_fov = mypos->isInFov(ppos->getPosition(), fov);
  if (!in_fov || VEC3::Distance(mypos->getPosition(), ppos->getPosition()) > distToChase + 0.5f) {
	  ChangeState("closestWpt");
  }

  if (VEC3::Distance(mypos->getPosition(), ppos->getPosition()) <= distToIdleWar) {
	  idleWarTimerMax = idleWarTimerBase + (rand() % idleWarTimerExtra);
	  ChangeState("idleWar");
  }
*/
}

void CAIPatrol::AttackState(float dt)
{
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

void CAIPatrol::BeginAlertState(float dt)
{
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
}

void CAIPatrol::SeekPlayerState(float dt)
{
}

void CAIPatrol::StunnedState(float dt)
{
}

void CAIPatrol::FixedState(float dt)
{
}

void CAIPatrol::ShadowMergedState(float dt)
{
}


void CAIPatrol::ChooseOrbitSideState(float dt) {
	/*orbitRotationMax = orbitRotationBase + deg2rad(rand() % orbitRotationExtra);
	if (rand() % 2 == 0) {
		ChangeState("orbitLeft");
	}
	else {
		ChangeState("orbitRight");
	}*/
}


void CAIPatrol::OrbitLeftState(float dt) {
	/*TCompTransform *mypos = getMyTransform();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	if (VEC3::Distance(mypos->getPosition(), ppos->getPosition()) <= distToBack) {
		currentOrbitRotation = 0;
		ChangeState("back");
	}
	else {
		float distance = VEC3::Distance(ppos->getPosition(), mypos->getPosition());
		float y, r, p;
		mypos->getYawPitchRoll(&y, &p, &r);

		mypos->setPosition(ppos->getPosition());
		float deltaYaw = (rotationSpeed * dt);
		currentOrbitRotation = currentOrbitRotation + deltaYaw;
		mypos->setYawPitchRoll(y - deltaYaw, p, r);

		mypos->setPosition(mypos->getPosition() - mypos->getFront() * distance);

		if (currentOrbitRotation >= orbitRotationMax) {
			currentOrbitRotation = 0;
			idleWarTimerMax = idleWarTimerBase + (rand() % idleWarTimerExtra);
			ChangeState("idleWar");
		}
	}*/
}


void CAIPatrol::OrbitRightState(float dt) {
	/*TCompTransform *mypos = getMyTransform();
	CEntity *player = (CEntity *)getEntityByName(entityToChase);
	TCompTransform *ppos = player->get<TCompTransform>();

	if (VEC3::Distance(mypos->getPosition(), ppos->getPosition()) <= distToBack) {
		currentOrbitRotation = 0;
		ChangeState("back");
	}
	else {
		float distance = VEC3::Distance(ppos->getPosition(), mypos->getPosition());
		float y, r, p;
		mypos->getYawPitchRoll(&y, &p, &r);

		mypos->setPosition(ppos->getPosition());
		float deltaYaw = (rotationSpeed * dt);
		currentOrbitRotation = currentOrbitRotation + deltaYaw;
		mypos->setYawPitchRoll(y + deltaYaw, p, r);

		mypos->setPosition(mypos->getPosition() - mypos->getFront() * distance);

		if (currentOrbitRotation >= orbitRotationMax) {
			currentOrbitRotation = 0;
			idleWarTimerMax = idleWarTimerBase + (rand() % idleWarTimerExtra);
			ChangeState("idleWar");
		}
	}*/
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


void CAIPatrol::HitState(float dt) {
	/* life = life - 1;
	 if (life <= 0) {
		ChangeState("dead");
	 }
	 else {
		 ChangeState("closestWpt");
	 }*/
}

void CAIPatrol::DeadState(float dt) {
	fatal("You are dead madafaka");
	exit(ERROR_VIRUS_INFECTED);
}


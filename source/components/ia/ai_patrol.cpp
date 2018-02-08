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
  AddState("seekwpt", (statehandler)&CAIPatrol::SeekWptState);
  AddState("nextwpt", (statehandler)&CAIPatrol::NextWptState);
  AddState("closestwpt", (statehandler)&CAIPatrol::ClosestWptState);
  AddState("chase", (statehandler)&CAIPatrol::ChaseState);
  AddState("idleWar", (statehandler)&CAIPatrol::IdleWarState);
  AddState("chooseOrbitSide", (statehandler)&CAIPatrol::ChooseOrbitSideState);
  AddState("orbitLeft", (statehandler)&CAIPatrol::OrbitLeftState);
  AddState("orbitRight", (statehandler)&CAIPatrol::OrbitRightState);
  AddState("back", (statehandler)&CAIPatrol::BackState);
  AddState("hit", (statehandler)&CAIPatrol::HitState);
  AddState("dead", (statehandler)&CAIPatrol::DeadState);

  // reset the state
  ChangeState("idle");
  currentWaypoint = 0;
}

void CAIPatrol::debugInMenu() {
  IAIController::debugInMenu();
  if (ImGui::TreeNode("Waypoints")) {
    for (auto& v : _waypoints) {
      ImGui::PushID(&v);
      ImGui::DragFloat3("Point", &v.x, 0.1f, -20.f, 20.f);
      ImGui::PopID();
    }
    ImGui::TreePop();
  }

  for (size_t i = 0; i < _waypoints.size(); ++i)
    renderLine(_waypoints[i], _waypoints[(i + 1) % _waypoints.size()], VEC4(0, 1, 0, 1));
}

void CAIPatrol::load(const json& j, TEntityParseContext& ctx) {
  setEntity(ctx.current_entity);

  Init();

  auto& j_waypoints = j["waypoints"];
  for (auto it = j_waypoints.begin(); it != j_waypoints.end(); ++it) {
    VEC3 p = loadVEC3(it.value());
    addWaypoint(p);
  }

}

void CAIPatrol::registerMsgs() {
	DECL_MSG(CAIPatrol, TMsgDamage, onMsgDamage);
}

void CAIPatrol::onMsgDamage(const TMsgDamage& msg) {
	ChangeState("hit");
}


void CAIPatrol::IdleState(float dt)
{
  CEntity *player = (CEntity *)getEntityByName(entityToChase);
  TCompTransform *mypos = getMyTransform();
  TCompTransform *ppos = player->get<TCompTransform>();
  bool in_fov = mypos->isInFov(ppos->getPosition(), deg2rad(60));
  if (in_fov) ChangeState("seekwpt");
}


void CAIPatrol::SeekWptState(float dt)
{
  TCompTransform *mypos = getMyTransform();
  float y, r, p;
  mypos->getYawPitchRoll(&y, &p, &r);
  if (mypos->isInLeft(getWaypoint()))
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
  vp = vp + speed * dt *vfwd;
  mypos->setPosition(vp);
  // next wpt
  if (VEC3::Distance(getWaypoint(), vp) < 1) ChangeState("nextwpt");

  // chase
  CEntity *player = (CEntity *)getEntityByName(entityToChase);
  TCompTransform *ppos = player->get<TCompTransform>();
  bool in_fov = mypos->isInFov(ppos->getPosition(), fov);
  if (in_fov) ChangeState("chase");
}

void CAIPatrol::NextWptState(float dt)
{
  currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
  ChangeState("seekwpt");
}


void CAIPatrol::ClosestWptState(float dt) {
	float minDistance = INFINITY;
	int  minIndexWpt = 0;

	for (int i = 0; i < _waypoints.size(); i++) {
		float currDistance = VEC3::Distance(getMyTransform()->getPosition(), _waypoints[i]);
		if (currDistance < minDistance) {
			minDistance = currDistance;
			minIndexWpt = i;
		}
	}

	currentWaypoint = minIndexWpt;
	ChangeState("seekwpt");
}


void CAIPatrol::ChaseState(float dt)
{
  TCompTransform *mypos = getMyTransform();
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
  if (!in_fov) ChangeState("closestwpt");

  if (VEC3::Distance(mypos->getPosition(), ppos->getPosition()) <= distToIdleWar) {
	  idleWarTimerMax = idleWarTimerBase + (rand() % idleWarTimerExtra);
	  ChangeState("idleWar");
  }

}


void CAIPatrol::IdleWarState(float dt) {
	TCompTransform *mypos = getMyTransform();
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
	}
}


void CAIPatrol::ChooseOrbitSideState(float dt) {
	orbitRotationMax = orbitRotationBase + deg2rad(rand() % orbitRotationExtra);
	if (rand() % 2 == 0) {
		ChangeState("orbitLeft");
	}
	else {
		ChangeState("orbitRight");
	}

}


void CAIPatrol::OrbitLeftState(float dt) {
	TCompTransform *mypos = getMyTransform();
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
	}
}


void CAIPatrol::OrbitRightState(float dt) {
	TCompTransform *mypos = getMyTransform();
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
	}
}


void CAIPatrol::BackState(float dt) {
	TCompTransform *mypos = getMyTransform();
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
	}
}


void CAIPatrol::HitState(float dt) {
	 life = life - 1;
	 if (life <= 0) {
		ChangeState("dead");
	 }
	 else {
		 ChangeState("closestwpt");
	 }
}

void CAIPatrol::DeadState(float dt) {
	fatal("You are dead madafaka");
	exit(ERROR_VIRUS_INFECTED);
}


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
  AddState("chase", (statehandler)&CAIPatrol::ChaseState);

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


void CAIPatrol::IdleState()
{
  CEntity *player = (CEntity *)getEntityByName("The Player");
  TCompTransform *mypos = getMyTransform();
  TCompTransform *ppos = player->get<TCompTransform>();
  bool in_fov = mypos->isInFov(ppos->getPosition(), deg2rad(60));
  if (in_fov) ChangeState("seekwpt");
}


void CAIPatrol::SeekWptState()
{
  TCompTransform *mypos = getMyTransform();
  float y, r, p;
  mypos->getYawPitchRoll(&y, &p, &r);
  if (mypos->isInLeft(getWaypoint()))
  {
    y += 0.001f;
  }
  else
  {
    y -= 0.001f;
  }
  mypos->setYawPitchRoll(y, p, r);
  VEC3 vp = mypos->getPosition();
  VEC3 vfwd = mypos->getFront();
  vfwd.Normalize();
  vp = vp + 0.001f*vfwd;
  mypos->setPosition(vp);
  // next wpt
  if (VEC3::Distance(getWaypoint(), vp) < 1) ChangeState("nextwpt");

  // chase
  CEntity *player = (CEntity *)getEntityByName("The Player");
  TCompTransform *ppos = player->get<TCompTransform>();
  bool in_fov = mypos->isInFov(ppos->getPosition(), deg2rad(60));
  if (in_fov) ChangeState("chase");
}

void CAIPatrol::NextWptState()
{
  currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
  ChangeState("seekwpt");
}

void CAIPatrol::ChaseState()
{
  TCompTransform *mypos = getMyTransform();
  CEntity *player = (CEntity *)getEntityByName("The Player");
  TCompTransform *ppos = player->get<TCompTransform>();
  float y, r, p;
  mypos->getYawPitchRoll(&y, &p, &r);
  if (mypos->isInLeft(ppos->getPosition()))
  {
    y += 0.001f;
  }
  else
  {
    y -= 0.001f;
  }
  mypos->setYawPitchRoll(y, p, r);
  VEC3 vp = mypos->getPosition();
  VEC3 vfwd = mypos->getFront();
  vfwd.Normalize();
  vp = vp + 0.001f*vfwd;
  mypos->setPosition(vp);

  bool in_fov = mypos->isInFov(ppos->getPosition(), deg2rad(60));
  if (!in_fov) ChangeState("seekwpt");

}


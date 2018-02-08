#ifndef _AIC_PATROL
#define _AIC_PATROL

#include "ai_controller.h"
#include "entity/common_msgs.h"

class CAIPatrol : public IAIController
{

	/* Atributes */
  std::vector<VEC3> _waypoints;
  int currentWaypoint;
  float speed = 2.f;
  float rotationSpeed = deg2rad(90);
  float fov = deg2rad(60);
  float distToIdleWar = 2.f;
  float distToBack = 1.f;
  std::string entityToChase = "The Player";
  int life = 5;

  /* Timers */
  float idleWarTimer = 0.f;
  float idleWarTimerBase = 1.f;
  int idleWarTimerExtra = 2;
  float idleWarTimerMax = 0.f;
  float currentOrbitRotation = 0.f;
  float orbitRotationBase = deg2rad(60);
  int orbitRotationExtra = 30;
  float orbitRotationMax = 0.f;

  DECL_SIBLING_ACCESS();

  void onMsgDamage(const TMsgDamage& msg);

public:
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();

  void IdleState(float);
  void SeekWptState(float);
  void NextWptState(float);
  void ClosestWptState(float);
  void ChaseState(float);
  void IdleWarState(float);
  void ChooseOrbitSideState(float);
  void OrbitLeftState(float);
  void OrbitRightState(float);
  void BackState(float);
  void HitState(float);
  void DeadState(float);

  void Init();

  void addWaypoint(VEC3 waypoint) { _waypoints.push_back(waypoint); };
  VEC3 getWaypoint() { return _waypoints[currentWaypoint]; }

  static void registerMsgs();
};

#endif
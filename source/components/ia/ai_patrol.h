#ifndef _AIC_PATROL
#define _AIC_PATROL

#include "ai_controller.h"
#include "entity/common_msgs.h"

class CAIPatrol : public IAIController
{

	/* Atributes */
  std::vector<VEC3> _waypoints;
  int currentWaypoint;
  float speed;
  float rotationSpeed;
  float fov;
  float distToIdleWar;
  float distToBack;
  float distToChase;
  std::string entityToChase;
  int life;

  /* Timers */
  float idleWarTimer = 0.f;
  float idleWarTimerMax = 0.f;
  float idleWarTimerBase;
  int idleWarTimerExtra;
  float currentOrbitRotation = 0.f;
  float orbitRotationMax = 0.f;
  float orbitRotationBase;
  int orbitRotationExtra;

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
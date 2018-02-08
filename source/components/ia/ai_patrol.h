#ifndef _AIC_PATROL
#define _AIC_PATROL

#include "ai_controller.h"

class CAIPatrol : public IAIController
{
  std::vector<VEC3> _waypoints;
  int currentWaypoint;

  DECL_SIBLING_ACCESS();

public:
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();

  void IdleState(float);
  void SeekWptState(float);
  void NextWptState(float);
  void ChaseState(float);

  void Init();

  void addWaypoint(VEC3 waypoint) { _waypoints.push_back(waypoint); };
  VEC3 getWaypoint() { return _waypoints[currentWaypoint]; }
};

#endif
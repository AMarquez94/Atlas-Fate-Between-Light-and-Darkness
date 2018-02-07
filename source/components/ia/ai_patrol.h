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

  void IdleState();
  void SeekWptState();
  void NextWptState();
  void ChaseState();

  void Init();

  void addWaypoint(VEC3 waypoint) { _waypoints.push_back(waypoint); };
  VEC3 getWaypoint() { return _waypoints[currentWaypoint]; }
};

#endif
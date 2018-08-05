#pragma once

#include "modules/module.h"
#include "entity/entity.h"

class IAIController;

struct PatrolSharedBoard {
	std::vector<CHandle> stunnedPatrols;
    std::vector<CHandle> patrolsWithLight;
};

struct Waypoint {
	VEC3 position;
	VEC3 lookAt;				//TODO: guess how to make the waypoint orientated to something
	float minTime;
};

class CModuleIA : public IModule
{
public:
  CModuleIA(const std::string& aname) : IModule(aname) { }
  void update(float delta) override;
  void render() override;
  void clearSharedBoards();

  /* SharedBoards */
  PatrolSharedBoard patrolSB;
};


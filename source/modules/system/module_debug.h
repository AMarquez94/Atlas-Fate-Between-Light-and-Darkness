#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include "navmesh/navmesh.h"
#include "navmesh/navmesh_query.h"

class CModuleDebug : public IModule
{

public:

	CModuleDebug(const std::string& aname) : IModule(aname) { }

  bool start() override;
  bool stop() override;
  void update(float delta) override;
  void render() override;

private:
    VEC3 p1 = VEC3::Zero;
    VEC3 p2 = VEC3::Zero;
    std::vector<VEC3> navmeshPath;
    bool recalculateNavmesh = true;
};


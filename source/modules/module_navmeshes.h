#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include "navmesh/navmesh.h"
#include "navmesh/navmesh_query.h"

class CModuleNavmesh : public IModule
{

public:

  CModuleNavmesh(const std::string& aname) : IModule(aname) { }

  bool start() override;
  bool stop() override;
  void update(float delta) override;
  void render() override;
  void buildNavmesh(const std::string& path);
  void destroyNavmesh();

  const std::vector<VEC3> findPath(VEC3 start, VEC3 end);
  float wallDistance(VEC3 pos);
  bool raycast(VEC3 start, VEC3 end, VEC3& hitPos);
  VEC3 closestNavmeshPoint(VEC3 start);
  bool renderNamvesh = false;
  float navmeshLong(const std::vector<VEC3> navmeshPath);

private:

  CNavmesh navmesh;
  CNavmeshQuery navmeshQuery = CNavmeshQuery(&navmesh);
  bool navmeshCreated;
};


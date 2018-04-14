#pragma once

#include "modules/module.h"
#include "entity/entity.h"

class CModuleNavmesh : public IModule
{
public:
  CModuleNavmesh(const std::string& aname) : IModule(aname) { }
  void update(float delta) override;
  void render() override;
};


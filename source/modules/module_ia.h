#pragma once

#include "modules/module.h"
#include "entity/entity.h"

class IAIController;

class CModuleIA : public IModule
{
public:
  CModuleIA(const std::string& aname) : IModule(aname) { }
  void update(float delta) override;
  void render() override;
};


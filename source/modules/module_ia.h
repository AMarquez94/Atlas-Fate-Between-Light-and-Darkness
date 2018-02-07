#pragma once

#include "modules/module.h"
#include "entity/entity.h"

class IAIController;

class CModuleIA : public IModule
{
  //std::vector< IAIController* > controllers;

public:
  CModuleIA(const std::string& aname) : IModule(aname) { }
  void update(float delta) override;
  void render() override;
  IAIController* getNewAIControler(const json& j, CHandle h_ia);
};


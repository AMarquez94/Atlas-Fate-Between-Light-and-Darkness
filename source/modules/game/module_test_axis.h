#pragma once

#include "modules/module.h"

class CModuleTestAxis : public IModule
{
public:
  CModuleTestAxis(const std::string& aname) : IModule(aname) { }
  bool start() override;
  bool stop() override;
  void update(float delta) override;
  void render() override;
};
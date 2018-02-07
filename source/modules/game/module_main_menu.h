#pragma once

#include "modules/module.h"

class CModuleMainMenu : public IModule
{
public:CModuleMainMenu(const std::string& name)
	: IModule(name)
{}
  bool start() override;
  void update(float delta) override;
};
#pragma once

#include "modules/module.h"

class CModuleGameOver : public IModule
{
public:
	CModuleGameOver(const std::string& name)
		: IModule(name)
	{}
  bool start() override;
  void update(float delta) override;
};
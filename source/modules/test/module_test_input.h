#pragma once

#include "modules/module.h"

class CModuleTestInput : public IModule
{
public:
	CModuleTestInput(const std::string& name)
		: IModule(name)
	{}
  bool start() override;
  void update(float delta) override;
};
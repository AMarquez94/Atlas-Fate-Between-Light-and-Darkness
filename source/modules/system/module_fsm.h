#pragma once

#include "modules/module.h"
#include "fsm/factory.h"

class CModuleFSM : public IModule
{
public:
  CModuleFSM(const std::string& name);
  bool start() override;
  bool stop() override;

  FSM::CFactory& getFactory() { return _factory; }
	//void loadButtonDefinitions(const std::string& filename);

private:
	void renderInMenu();

  FSM::CFactory _factory;
};

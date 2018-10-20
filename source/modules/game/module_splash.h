#pragma once

#include "modules/module.h"

class CModuleSplash : public IModule
{
public:
	CModuleSplash(const std::string& name)
		: IModule(name)
	{}
  bool start() override;
  bool stop() override;
  void update(float delta) override;

private:
  float _timer;
  float _max_time;

  bool upfFadedOut = false;
  bool babyRobotFadedOut = false;
  bool softwareFadedOut = false; 
  bool engineFadedOut = false;
};
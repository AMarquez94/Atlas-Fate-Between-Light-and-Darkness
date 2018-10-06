#pragma once

#include "modules/module.h"

class CModuleCredits : public IModule
{

	float time;

public:
	float transition_speed = 0.5f;

	CModuleCredits(const std::string& name) : IModule(name) {}
	bool start() override;
	bool stop() override;
	void update(float delta) override;
	void render() override;
};
#pragma once

#include "modules/module.h"

class CModuleMainMenu : public IModule
{
	ImGuiWindowFlags window_flags;
	unsigned int window_width;
	unsigned int window_height;

	UINT32 level_load = 0;
	float time;
	float transition_speed = 0.5f;

public:

	public:CModuleMainMenu(const std::string& name): IModule(name) {}
	bool start() override;
	void update(float delta) override;
};
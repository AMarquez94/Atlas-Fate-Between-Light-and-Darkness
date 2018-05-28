#pragma once

#include "modules/module.h"

class CModuleMainMenu : public IModule
{
	ImGuiWindowFlags window_flags;
	unsigned int window_width;
	unsigned int window_height;
	unsigned menu_load = 0;
	const unsigned int menu_size = 6;

	float time;

public:
	float transition_speed = 0.5f;

	CModuleMainMenu(const std::string& name): IModule(name) {}
	bool start() override;
	bool stop() override;
	void update(float delta) override;
	void render() override;
};
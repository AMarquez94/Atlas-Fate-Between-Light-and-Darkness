#pragma once

#include "modules/module.h"

class CModuleLevelSelect : public IModule
{
	// Menu window related variables.
	ImGuiWindowFlags window_flags;
	unsigned int window_width;
	unsigned int window_height;
	unsigned int menuPosition = 0;
	const unsigned int menuSize = 3;

	float time;

public:

	float transition_speed = 0.5f;

	CModuleLevelSelect(const std::string& name): IModule(name) {}

	bool start() override;
	void update(float delta) override;
	void render() override;
};
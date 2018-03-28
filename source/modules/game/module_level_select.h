#pragma once

#include "modules/module.h"

class CModuleLevelSelect : public IModule
{
	// Menu window related variables.
	ImGuiWindowFlags window_flags;
	unsigned int window_width;
	unsigned int window_height;
	unsigned int menuPosition = 0;
	const unsigned int menuSize = 2;

	bool isPaused;
	bool menuVisible;
	bool victoryMenuVisible;

public:

	CModuleLevelSelect(const std::string& name): IModule(name) {}

	bool start() override;
	void update(float delta) override;
	void render() override;
};
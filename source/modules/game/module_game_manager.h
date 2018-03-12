#pragma once

#include "modules/module.h"

class CModuleGameManager : public IModule
{
	// Menu window related variables.
	ImGuiWindowFlags window_flags;
	unsigned int window_width;
	unsigned int window_height;
	const unsigned int menuSize = 2;
	unsigned int menuPosition = 0;

	bool isPaused;
	CHandle player;
	bool menuVisible;
	bool victoryMenuVisible;

public:

	public:CModuleGameManager(const std::string& name): IModule(name) {}
	bool start() override;
	void update(float delta) override;
	void render() override;
};
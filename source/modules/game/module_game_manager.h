#pragma once

#include "modules/module.h"

class CModuleGameManager : public IModule
{
	/* Mantain a handle of the player */
	CHandle player;

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

	CModuleGameManager(const std::string& name): IModule(name) {}

	bool start() override;
	void update(float delta) override;
	void render() override;
};
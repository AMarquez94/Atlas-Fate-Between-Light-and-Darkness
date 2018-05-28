#pragma once

#include "modules/module.h"
#include "checkpoints/checkpoint.h"

class CModuleGameManager : public IModule
{
	/* Mantain a handle of the player */
	CHandle player;

	// Menu window related variables.
	ImGuiWindowFlags window_flags;
	unsigned int window_width;
	unsigned int window_height;
	unsigned int menuPosition = 0;
	const unsigned int menuSize = 4;

	CCheckpoint* lastCheckpoint;

	bool isPaused;
	bool menuVisible;
	bool victoryMenuVisible;
	bool playerDiedMenuVisible;

	bool isStarted = false;

public:

    struct ConfigPublic {
        bool drawfps = true;

    }config;

	CModuleGameManager(const std::string& name): IModule(name) {}

	bool start() override;
	void update(float delta) override;
	void render() override;

	bool saveCheckpoint(VEC3 playerPos, QUAT playerRot);
	bool loadCheckpoint();
	bool deleteCheckpoint();

  void unpauseGame();

  void debugRender();
};
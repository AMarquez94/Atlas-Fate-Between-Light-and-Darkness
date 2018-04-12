#pragma once

#include "modules/module.h"

class CModuleGameConsole : public IModule
{

	// Menu window related variables.
	ImGuiWindowFlags window_flags;
	ImGuiInputTextFlags console_flags;
	unsigned int window_width;
	unsigned int window_height;

	bool consoleVisible;

	std::vector<std::string> historicCommands;
	std::vector<std::string> allCommands;

	int historicCommandsPos = -1;
	char command[128] = "";

	static int ConsoleBehaviourCallbackStub(ImGuiTextEditCallbackData* data);

public:

	CModuleGameConsole(const std::string& name): IModule(name) {}

	bool start() override;
	void update(float delta) override;
	void render() override;

	int ConsoleBehaviourCallback(ImGuiTextEditCallbackData* data);
};
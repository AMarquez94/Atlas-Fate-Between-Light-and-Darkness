#pragma once

#include "modules/module.h"

class CModuleGameConsole : public IModule
{

	struct OutputString {
		ImVec4 color;
		std::string text;
	};

	// Menu window related variables.
	ImGuiWindowFlags window_console_flags;
	ImGuiWindowFlags window_output_flags;
	ImGuiInputTextFlags console_flags;
	unsigned int console_height;
	unsigned int output_height;

	bool consoleVisible;
	bool outputVisible;

	bool outputLockCursorAtBottom = true;
	float offsetOutputCursor = 0.f;
	float scrollDeltaDistance;

	std::vector<std::string> historicCommands;
	std::vector<std::string> allCommands;

	int historicCommandsPos = -1;

	#define COMMAND_SIZE 128
	//#define OUTPUT_SIZE 1024 * 16

	char command[COMMAND_SIZE] = "";
	std::vector<OutputString> output;
	//char output[OUTPUT_SIZE] = "esto es una prueba\n de esta wea a ver\n si funciona carallo\n porque estoy rellenado mucho hueco\n y no se si funcionara\n eu que sei mon amour\n pues alguna frase\n mas me faltaba\n a ver si ahora si";

	static int ConsoleBehaviourCallbackStub(ImGuiTextEditCallbackData* data);

public:

	CModuleGameConsole(const std::string& name): IModule(name) {}

	bool start() override;
	void update(float delta) override;
	void render() override;

	int ConsoleBehaviourCallback(ImGuiTextEditCallbackData* data);
};
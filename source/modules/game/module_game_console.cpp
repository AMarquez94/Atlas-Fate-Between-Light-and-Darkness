#include "mcv_platform.h"
#include "module_game_console.h"
#include "input/devices/mouse.h"
#include "components/comp_tags.h"
#include "entity/common_msgs.h"
#include "entity/msgs.h"
#include "modules/module_entities.h"
#include "windows/app.h"

bool CModuleGameConsole::start()
{
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoTitleBar;

	console_flags = 0;
	console_flags |= ImGuiInputTextFlags_CallbackHistory;
	console_flags |= ImGuiInputTextFlags_CallbackCompletion;
	console_flags |= ImGuiInputTextFlags_CallbackCharFilter;
	console_flags |= ImGuiInputTextFlags_EnterReturnsTrue;

	return true;
}

void CModuleGameConsole::update(float delta)
{
	if (EngineInput["btConsole"].getsPressed()) {
		consoleVisible = !consoleVisible;
		*command = 0;
		historicCommandsPos = historicCommands.size();
	}
}

void CModuleGameConsole::render()
{
	VEC2 menu_position = VEC2(0,0);

	if (consoleVisible) {
		ImGui::SetNextWindowSize(ImVec2(Render.width,20));
		ImGui::Begin("Console", false, window_flags);
		ImGui::CaptureMouseFromApp(false);
		ImGui::SetWindowPos("Console", ImVec2(menu_position.x, menu_position.y));
	/*	if (ImGui::InputText("command", str0, IM_ARRAYSIZE(str0), console_flags, &TextEditCallbackStub)) {
			dbg(str0);
		}*/
		ImGui::PushItemWidth(-1);
		if (ImGui::InputText("Console", command, IM_ARRAYSIZE(command), console_flags, ConsoleBehaviourCallbackStub, (void *)this)) {
			std::string commandString = command;
			if (commandString.size() > 0) {
				dbg(command);
				historicCommands.push_back(commandString);
				dbg("\n");
				*command = 0;
				historicCommandsPos = historicCommands.size();
			}
		}
		ImGui::PopItemWidth();
		ImGui::SetKeyboardFocusHere();
		ImGui::End();
	}
}

int CModuleGameConsole::ConsoleBehaviourCallback(ImGuiTextEditCallbackData * data)
{
	const int flagType = data->EventFlag;

	switch (flagType) {
	case ImGuiInputTextFlags_CallbackHistory:

		if (data->EventKey == ImGuiKey_UpArrow) {
			historicCommandsPos = Clamp(historicCommandsPos - 1, 0, (int)historicCommands.size());
		}
		else if (data->EventKey == ImGuiKey_DownArrow) {
			historicCommandsPos = Clamp(historicCommandsPos + 1, 0, (int)historicCommands.size());
		}

		data->CursorPos = data->SelectionStart = data->SelectionEnd =  data->BufTextLen = 
			(int)snprintf(data->Buf, (size_t)data->BufSize, "%s", historicCommandsPos < historicCommands.size() ? historicCommands[historicCommandsPos].c_str() : "");
		data->BufDirty = true;

		break;

	case ImGuiInputTextFlags_CallbackCompletion:

		break;
	}

	if (strchr("º", (char)data->EventChar)) {
		return 1;
	}
	else {
		return 0;
	}
}

int CModuleGameConsole::ConsoleBehaviourCallbackStub(ImGuiTextEditCallbackData * data)
{
	CModuleGameConsole* console = (CModuleGameConsole*)data->UserData;
	return console->ConsoleBehaviourCallback(data);
}

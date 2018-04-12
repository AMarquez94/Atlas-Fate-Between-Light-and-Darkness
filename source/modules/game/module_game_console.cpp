#include "mcv_platform.h"
#include "module_game_console.h"
#include "input/devices/mouse.h"
#include "components/comp_tags.h"
#include "entity/common_msgs.h"
#include "entity/msgs.h"
#include "modules/module_entities.h"
#include "windows/app.h"
#include "utils/utils.h"

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

	allCommands.push_back("hola");
	allCommands.push_back("prueba");
	allCommands.push_back("pruebendo");
	allCommands.push_back("equisde");

	return true;
}

void CModuleGameConsole::update(float delta)
{
	if (EngineInput["btConsole"].getsPressed()) {
		consoleVisible = !consoleVisible;
		*command = 0;
		historicCommandsPos = historicCommands.size();

		CHandle hPlayer = getEntityByName("The Player");
		CEntity * ePlayer = hPlayer;
		TMsgConsoleOn msg;
		msg.isConsoleOn = consoleVisible;
		ePlayer->sendMsg(msg);
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
		ImGui::PushItemWidth(-1);
		if (ImGui::InputText("Console", command, IM_ARRAYSIZE(command), console_flags, ConsoleBehaviourCallbackStub, (void *)this)) {
			std::string commandString = command;
			if (commandString.size() > 0) {

				/* Ejecutar LUA */
				dbg("%s\n",command);
				historicCommands.push_back(commandString);
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

			data->CursorPos = 
			data->SelectionStart = 
			data->SelectionEnd =  
			data->BufTextLen = (int)snprintf(data->Buf, 
				(size_t)data->BufSize, 
				"%s", historicCommandsPos < historicCommands.size() ? historicCommands[historicCommandsPos].c_str() : "");
			data->BufDirty = true;

		break;

		case ImGuiInputTextFlags_CallbackCompletion:

			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}

			// Build a list of candidates
			ImVector<const char*> candidates;
			for (int i = 0; i < allCommands.size(); i++)
				if (Strnicmp(allCommands[i].c_str(), word_start, (int)(word_end - word_start)) == 0)
					candidates.push_back(allCommands[i].c_str());

			if (candidates.Size == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
			}
			else if(candidates.Size > 1)
			{
				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}
			}

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
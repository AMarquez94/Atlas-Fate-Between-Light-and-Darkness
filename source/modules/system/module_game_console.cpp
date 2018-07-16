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
    window_console_flags = 0;
    window_console_flags |= ImGuiWindowFlags_NoMove;
    window_console_flags |= ImGuiWindowFlags_NoResize;
    window_console_flags |= ImGuiWindowFlags_NoCollapse;
    window_console_flags |= ImGuiWindowFlags_NoScrollbar;
    window_console_flags |= ImGuiWindowFlags_NoTitleBar;

    window_output_flags = 0;
    window_output_flags |= ImGuiWindowFlags_NoMove;
    window_output_flags |= ImGuiWindowFlags_NoResize;
    window_output_flags |= ImGuiWindowFlags_NoCollapse;
    window_output_flags |= ImGuiWindowFlags_NoTitleBar;

    console_flags = 0;
    console_flags |= ImGuiInputTextFlags_CallbackHistory;
    console_flags |= ImGuiInputTextFlags_CallbackCompletion;
    console_flags |= ImGuiInputTextFlags_CallbackCharFilter;
    console_flags |= ImGuiInputTextFlags_EnterReturnsTrue;

    allCommands.push_back("hola");
    allCommands.push_back("prueba");
    allCommands.push_back("pruebendo");
    allCommands.push_back("equisde");

    console_height = 20;
    output_height = 160;
    scrollDeltaDistance = 65.f;

    return true;
}

void CModuleGameConsole::update(float delta)
{
    if (EngineInput["btConsole"].getsPressed()) {
        consoleVisible = !consoleVisible;
        *command = 0;
        historicCommandsPos = (int)historicCommands.size();

        CHandle hPlayer = getEntityByName("The Player");
        if (hPlayer.isValid()) {
            CEntity * ePlayer = hPlayer;
            TMsgConsoleOn msg;
            msg.isConsoleOn = consoleVisible;
            ePlayer->sendMsg(msg);
        }

        if (!consoleVisible) {
            outputVisible = false;
        }
    }

    if (outputVisible) {
        if (EngineInput.mouse()._wheel_delta != 0) {
            outputLockCursorAtBottom = false;
            offsetOutputCursor = EngineInput.mouse()._wheel_delta * scrollDeltaDistance;
        }
    }
}

void CModuleGameConsole::renderMain()
{
    VEC2 menu_position = VEC2(0, 0);

    if (outputVisible) {
        ImGui::SetNextWindowSize(ImVec2((float)Render.width, (float)output_height));
        ImGui::Begin("Output", false, window_output_flags);
        ImGui::CaptureMouseFromApp(false);
        ImGui::SetWindowPos("Output", ImVec2(menu_position.x, menu_position.y));
        for (int i = 0; i < output.size(); i++) {
            ImGui::TextColored(output[i].color, output[i].text.c_str());
        }

        float scrollY = ImGui::GetScrollY();
        float maxScrollY = ImGui::GetScrollMaxY();

        scrollY = Clamp(scrollY - offsetOutputCursor, 0.f, maxScrollY);
        offsetOutputCursor = 0;

        if (scrollY == maxScrollY) {
            outputLockCursorAtBottom = true;
        }

        if (outputLockCursorAtBottom) {
            ImGui::SetScrollHere();
        }
        else {
            ImGui::SetScrollY(scrollY);
        }

        ImGui::End();
    }
    if (consoleVisible) {
        ImGui::SetNextWindowSize(ImVec2((float)Render.width, (float)console_height));
        ImGui::Begin("Console", false, window_console_flags);
        ImGui::CaptureMouseFromApp(false);
        ImGui::SetWindowPos("Console", ImVec2(menu_position.x, outputVisible ? menu_position.y + output_height : menu_position.y));
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("Console", command, IM_ARRAYSIZE(command), console_flags, ConsoleBehaviourCallbackStub, (void *)this)) {
            std::string commandString = command;
            if (commandString.size() > 0) {

                if (commandString.compare("expand") == 0) {
                    outputVisible = true;
                    *command = 0;
                }
                else if (commandString.compare("contract") == 0) {
                    outputVisible = false;
                    *command = 0;
                }
                else {
                    /* Ejecutar LUA - TODO: Filter reserved keywords */
                    Engine.getLogic().execCvar(commandString);
                    CModuleLogic::ConsoleResult result = Engine.getLogic().execScript(commandString);
                    historicCommands.push_back(commandString);
                    *command = 0;
                    historicCommandsPos = (int)historicCommands.size();

                    OutputString outString;

                    outString.text = "> " + commandString;
                    outString.color = ImVec4(1, 1, 1, 1);
                    output.emplace_back(outString);

                    outString.text = result.success ? "Success" : "Error";
                    outString.text = result.resultMsg.size() > 0 ? outString.text + "\n" + result.resultMsg : outString.text;
                    outString.color = result.success ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);
                    output.emplace_back(outString);
                }
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
        else if (candidates.Size > 1)
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

void CModuleGameConsole::addCommandToList(const std::string& command)
{
    allCommands.push_back(command);
}

int CModuleGameConsole::ConsoleBehaviourCallbackStub(ImGuiTextEditCallbackData * data)
{
    CModuleGameConsole* console = (CModuleGameConsole*)data->UserData;
    return console->ConsoleBehaviourCallback(data);
}

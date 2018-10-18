#include "mcv_platform.h"
#include "module_level_select.h"
#include "windows/app.h"
#include "input/devices/mouse.h"

bool CModuleLevelSelect::start()
{
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_width = 200;
	window_height = 150;

	Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));

	return true;
}

void CModuleLevelSelect::update(float delta)
{
	time += delta * transition_speed;

	if (EngineInput["btUpAux"].getsPressed()) {
		menuPosition = (menuPosition - 1) % menuSize;
	}

	if (EngineInput["btDownAux"].getsPressed()) {
		menuPosition = (menuPosition + 1) % menuSize;
	}
}

void CModuleLevelSelect::render()
{
	float background_value = abs(sin(time)) * 0.3f + 0.3f;
	CEngine::get().getRender().setBackgroundColor(background_value, background_value, background_value, 1.f);

	VEC2 menu_position = VEC2(CApp::get().xres * .5f - (window_width * .5f),
		CApp::get().yres * .5f - (window_height * .5f));

	ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height));
	ImGui::Begin("Level Select", false, window_flags);
	ImGui::CaptureMouseFromApp(false);
	ImGui::SetWindowPos("Level Select", ImVec2(menu_position.x, menu_position.y));
	ImGui::Selectable("Map Intro", menuPosition == 0);
	if (ImGui::IsItemClicked() || (menuPosition == 0 && EngineInput["btMenuConfirm"].getsPressed()))
	{
        EngineScene.changeGameState("map_intro");
	}

	ImGui::Selectable("Map Lights", menuPosition == 1);
	if (ImGui::IsItemClicked() || (menuPosition == 1 && EngineInput["btMenuConfirm"].getsPressed()))
	{
        EngineScene.changeGameState("map_intro");
    }
	ImGui::Selectable("Return Main Menu", menuPosition == 2);
	if (ImGui::IsItemClicked() || (menuPosition == 2 && EngineInput["btMenuConfirm"].getsPressed()))
	{
        EngineScene.changeGameState("map_intro");
    }


	ImGui::End();
}
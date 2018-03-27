#include "mcv_platform.h"
#include "module_game_manager.h"
#include "input/devices/mouse.h"
#include "components/comp_tags.h"
#include "entity/common_msgs.h"
#include "entity/msgs.h"
#include "modules/module_entities.h"
#include "windows/app.h"

bool CModuleGameManager::start()
{
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_width = 200;
	window_height = 150;

	isPaused = false;
	victoryMenuVisible = false;
	menuVisible = false;

	player = CTagsManager::get().getAllEntitiesByTag(getID("player"))[0];

	return true;
}

void CModuleGameManager::update(float delta)
{
	bool found = false;
	auto& handles = CTagsManager::get().getAllEntitiesByTag(getID("victory_trigger"));

	for (unsigned int i = 0; i < handles.size(); i++){

		CEntity* eCollider = handles[i];
		TCompCollider * e = eCollider->get<TCompCollider>();
		if (e->player_inside) {

			victoryMenuVisible = true;
			found = true;
			break;
		}
	}

	if (EngineInput["btPause"].getsPressed()) {

		// Send pause message
		TMsgScenePaused msg;
		msg.isPaused = !menuVisible;
		EngineEntities.broadcastMsg(msg);

		// Lock/Unlock the cursor
		Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
		ShowCursor(!menuVisible);
		mouse->setLockMouse();

		menuVisible = !menuVisible;
	}

	if (EngineInput["btUpAux"].getsPressed()){
		menuPosition = (menuPosition - 1) % menuSize;
	}

	if (EngineInput["btDownAux"].getsPressed()){
		menuPosition = (menuPosition + 1) % menuSize;
	}
}

void CModuleGameManager::render()
{
	VEC2 menu_position = VEC2(CApp::get().xres * .5f - (window_width * .5f),
		CApp::get().yres * .5f - (window_height * .5f));

	// Replace this with separated menus
	if (victoryMenuVisible) {

		ImGui::SetNextWindowSize(ImVec2(window_width, window_height));
		ImGui::Begin("VICTORY!", false, window_flags);
		ImGui::CaptureMouseFromApp(false);
		ImGui::SetWindowPos("VICTORY!", ImVec2(menu_position.x, menu_position.y));
		ImGui::Text("Enjoy your dopamine shot");
		ImGui::End();
	}
	else if (menuVisible) {

		ImGui::SetNextWindowSize(ImVec2(window_width, window_height));
		ImGui::Begin("MENU", false, window_flags);
		ImGui::CaptureMouseFromApp(false);
		ImGui::SetWindowPos("MENU", ImVec2(menu_position.x, menu_position.y));
		ImGui::Selectable("Restart game", menuPosition == 0);
		if (ImGui::IsItemClicked() || (menuPosition == 0 && EngineInput["btMenuConfirm"].getsPressed()))
		{
			dbg("Restarting the game\n");
			CEngine::get().getModules().changeGameState("map_intro");
		}

		ImGui::Selectable("Exit game", menuPosition == 1);
		if (ImGui::IsItemClicked() || (menuPosition == 1 && EngineInput["btMenuConfirm"].getsPressed()))
		{
			exit(0);
		}

		ImGui::End();
	}
}
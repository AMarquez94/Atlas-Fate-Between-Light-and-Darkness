#include "mcv_platform.h"
#include "comp_game_manager_controller.h"
#include "entity/entity_parser.h"
#include "windows/app.h"

DECL_OBJ_MANAGER("game_manager_controller", TCompGameManagerController);

void TCompGameManagerController::debugInMenu() {

}

void TCompGameManagerController::renderDebug()
{
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoScrollbar;

	if (menuVisible) {
		ImGui::SetNextWindowSize(ImVec2(100.f, 100.f));
		ImGui::Begin("Menu:", false, window_flags);
		ImGui::SetWindowPos("Menu:", ImVec2(CApp::get().xres/2, CApp::get().yres/2));
		if (ImGui::Button("Reset game"))
			CEngine::get().getModules().changeGameState("test_axis");
		ImGui::End();
	}
}

void TCompGameManagerController::load(const json& j, TEntityParseContext& ctx) {
	player = ctx.findEntityByName(j.value("player", "The Player"));
	menuVisible = false;
}

void TCompGameManagerController::update(float dt) {
	if (EngineInput["btPause"].getsPressed()) {
		menuVisible = !menuVisible;
	}
}
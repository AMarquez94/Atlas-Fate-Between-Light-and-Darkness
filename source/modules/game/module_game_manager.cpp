#include "mcv_platform.h"
#include "module_game_manager.h"
#include "input/devices/mouse.h"
#include "components/comp_tags.h"
#include "entity/common_msgs.h"
#include "entity/msgs.h"
#include "modules/module_entities.h"
#include "windows/app.h"
#include "components/player_controller/comp_player_tempcontroller.h"

bool CModuleGameManager::start()
{
    window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_width = 250;
    window_height = 150;

    isPaused = false;
    victoryMenuVisible = false;
    playerDiedMenuVisible = false;
    menuVisible = false;

    player = CTagsManager::get().getAllEntitiesByTag(getID("player"))[0];

    Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
    mouse->setLockMouse(true);

    isStarted = true;
    lastCheckpoint = new CCheckpoint();
		//lastCheckpoint.init();

    return true;
}

void CModuleGameManager::update(float delta)
{
	auto& handles = CTagsManager::get().getAllEntitiesByTag(getID("victory_trigger"));

	for (unsigned int i = 0; i < handles.size() && !victoryMenuVisible; i++) {

		CEntity* eCollider = handles[i];
		TCompCollider * e = eCollider->get<TCompCollider>();
		if (e && e->player_inside) {

			TMsgScenePaused msg;
			msg.isPaused = true;
			EngineEntities.broadcastMsg(msg);
			victoryMenuVisible = true;
		}
	}

    CEntity* e = player;
    if (e) {

        /* Player Dead */
        TCompTempPlayerController *playerCont = e->get<TCompTempPlayerController>();
        if (playerCont->isDead()) {
            Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
            mouse->setLockMouse(false);
            playerDiedMenuVisible = true;
            TMsgScenePaused msg;
            msg.isPaused = true;
            EngineCameras.getCurrentCamera().sendMsg(msg);
        }
    }

    if (EngineInput["btDebugParticles"].getsPressed()) {

        Engine.get().getParticles().particles_enabled = !Engine.get().getParticles().particles_enabled;
        Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
        mouse->setLockMouse(!Engine.get().getParticles().particles_enabled);

        TMsgScenePaused msg;
        msg.isPaused = Engine.get().getParticles().particles_enabled;
        EngineEntities.broadcastMsg(msg);
    }
    else if (!isPaused && EngineInput["btPause"].getsPressed() || (!menuVisible && CApp::get().lostFocus)) {

        /* Player not dead but game paused */

        isPaused = true;
        CApp::get().lostFocus = false;

        // Send pause message
        TMsgScenePaused msg;
        msg.isPaused = isPaused;
        EngineEntities.broadcastMsg(msg);

        // Lock/Unlock the cursor
        Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
        mouse->setLockMouse(false);

        menuVisible = true;
    }
    else if (isPaused && EngineInput["btPause"].getsPressed()) {

        /* Player not dead and game unpaused */
      unpauseGame();
    }

    if (EngineInput["btUpAux"].getsPressed()) {
        menuPosition = (menuPosition - 1) % menuSize;
    }

    if (EngineInput["btDownAux"].getsPressed()) {
        menuPosition = (menuPosition + 1) % menuSize;
    }

    if (EngineInput["btDebugMode"].getsPressed()) {
        EngineRender.setDebugMode(!EngineRender.getDebugMode());
    }
}

void CModuleGameManager::render()
{
    VEC2 menu_position = VEC2(CApp::get().xres * .5f - (window_width * .5f),
        CApp::get().yres * .5f - (window_height * .5f));

    // Replace this with separated menus
    if (menuVisible) {

        ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height));
        ImGui::Begin("MENU", false, window_flags);
        ImGui::CaptureMouseFromApp(false);
        ImGui::SetWindowPos("MENU", ImVec2(menu_position.x, menu_position.y));

        ImGui::Selectable("Resume game", menuPosition == 0);
        if (ImGui::IsItemClicked() || (menuPosition == 0 && EngineInput["btMenuConfirm"].getsPressed()))
        {
          unpauseGame();
        }

        ImGui::Selectable("Restart from last checkpoint", menuPosition == 1);
        if (ImGui::IsItemClicked() || (menuPosition == 1 && EngineInput["btMenuConfirm"].getsPressed()))
        {
            CEngine::get().getModules().changeGameState("map_intro");
        }

        ImGui::Selectable("Restart level", menuPosition == 2);
        if (ImGui::IsItemClicked() || (menuPosition == 2 && EngineInput["btMenuConfirm"].getsPressed()))
        {
          lastCheckpoint->deleteCheckPoint();
          CEngine::get().getModules().changeGameState("map_intro");
        }

        ImGui::Selectable("Exit game", menuPosition == 3);
        if (ImGui::IsItemClicked() || (menuPosition == 3 && EngineInput["btMenuConfirm"].getsPressed()))
        {
            exit(0);
        }

        ImGui::End();
    }
    else if (victoryMenuVisible) {

        ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height));
        ImGui::Begin("VICTORY!", false, window_flags);
        ImGui::CaptureMouseFromApp(false);
        ImGui::SetWindowPos("VICTORY!", ImVec2(menu_position.x, menu_position.y));
        ImGui::Text("Enjoy your dopamine shot");
        ImGui::End();

    }
    else if (playerDiedMenuVisible) {

        ImGui::SetNextWindowSize(ImVec2((float)window_width * 1.2f, (float)window_height));
        ImGui::Begin("YOU DIED! WHAT WOULD YOU DO?", false, window_flags);
        ImGui::CaptureMouseFromApp(false);
        ImGui::SetWindowPos("YOU DIED! WHAT WOULD YOU DO?", ImVec2(menu_position.x, menu_position.y));

        ImGui::Selectable("Restart from last checkpoint", menuPosition == 1);
        if (ImGui::IsItemClicked() || (menuPosition == 1 && EngineInput["btMenuConfirm"].getsPressed()))
        {
          CEngine::get().getModules().changeGameState("map_intro");
        }

        ImGui::Selectable("Restart level", menuPosition == 2);
        if (ImGui::IsItemClicked() || (menuPosition == 2 && EngineInput["btMenuConfirm"].getsPressed()))
        {
          lastCheckpoint->deleteCheckPoint();
          CEngine::get().getModules().changeGameState("map_intro");
        }

        ImGui::Selectable("Exit game", menuPosition == 3);
        if (ImGui::IsItemClicked() || (menuPosition == 3 && EngineInput["btMenuConfirm"].getsPressed()))
        {
          exit(0);
        }

        ImGui::End();

    }

    debugRender();
}

bool CModuleGameManager::saveCheckpoint(VEC3 playerPos, QUAT playerRot)
{
	return lastCheckpoint->saveCheckPoint(playerPos, playerRot);
}

bool CModuleGameManager::loadCheckpoint()
{
    if (lastCheckpoint) {
    return lastCheckpoint->loadCheckPoint();
    }
    else {
    return false;
    }
}

bool CModuleGameManager::deleteCheckpoint()
{
  if (lastCheckpoint) {
	  return lastCheckpoint->deleteCheckPoint();
  }
  else {
    return false;
  }
}

void CModuleGameManager::unpauseGame()
{
    /* Player not dead and game unpaused */
    isPaused = false;
    CApp::get().lostFocus = false;

    // Send pause message
    TMsgScenePaused msg;
    msg.isPaused = false;
    EngineEntities.broadcastMsg(msg);

    // Lock/Unlock the cursor
    Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
    mouse->setLockMouse(true);

    menuVisible = false;
}

void CModuleGameManager::debugRender() {

    if (lastCheckpoint) {
      lastCheckpoint->debugInMenu();
    }
    // Extra windows
    {
        //UI Window's Size
        ImGui::SetNextWindowSize(ImVec2((float)CApp::get().xres, (float)CApp::get().yres), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);

        ImGui::Begin("UI", NULL,
            ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_::ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);
        {
            ImGui::SetCursorPos(ImVec2(CApp::get().xres - CApp::get().xres * 0.05f, CApp::get().yres * 0.01f));

            if (config.drawfps) ImGui::Text("FPS %d", (int)CApp::get().fps);
        }

        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }
}
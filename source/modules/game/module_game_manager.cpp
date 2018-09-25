#include "mcv_platform.h"
#include "module_game_manager.h"
#include "input/devices/mouse.h"
#include "components/comp_tags.h"
#include "entity/common_msgs.h"
#include "entity/msgs.h"
#include "modules/module_entities.h"
#include "windows/app.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/camera_controller/comp_camera_flyover.h"

bool CModuleGameManager::start() {

    window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_width = 250;
    window_height = 150;

    _player = EngineEntities.getPlayerHandle();
    _fly_camera = getEntityByName("test_camera_flyover");
    //ambient = EngineSound.playEvent("event:/Ambiance/Intro_Ambiance");

    lastCheckpoint = new CCheckpoint();
    _currentstate = PauseState::none;
    return true;
}

void CModuleGameManager::setPauseState(PauseState pause) {

    // We are exiting the current state, disabling pause
    if (_currentstate == pause && CApp::get().hasFocus())
        pause = PauseState::none;

    {
        _player = EngineEntities.getPlayerHandle();
        _fly_camera = getEntityByName("test_camera_flyover");

        // Determine if whole scene is paused
        TMsgScenePaused msg;
        msg.isPaused = pause != PauseState::none && pause != PauseState::editor1unpaused ? true : false;
        EngineEntities.broadcastMsg(msg);
        EngineCameras.getCurrentCamera().sendMsg(msg);
        pauseEnemyEntities(msg.isPaused && pause != PauseState::defeat);
        EngineLogic.setPause(msg.isPaused);

        // Determine if player is paused
        CEntity * e_player = _player;
        CEntity * e_camera = _fly_camera;
        TCompCameraFlyover * flyover = e_camera->get<TCompCameraFlyover>();
        TMsgScenePaused msg2;
        msg2.isPaused = (!msg.isPaused && flyover->paused) ? false : true && pause != PauseState::defeat;
        e_player->sendMsg(msg2);
        dbg("current state %d and message %d\n", _currentstate, msg.isPaused);
    }
    switchState(pause);
}

void CModuleGameManager::switchState(PauseState pause) {

    // We set the new values for the given pause state
    // Send all the necessary messages too
    // Use this switch in case new logic is needed

    resetState();
    Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));

    switch (pause) {
    case PauseState::none: {
        mouse->setLockMouse(true);
    }break;
    case PauseState::main: {
        mouse->setLockMouse(false);
    }break;
    case PauseState::win: {
        mouse->setLockMouse(false);
    }break;
    case PauseState::defeat: {
        mouse->setLockMouse(false);
    }break;
    case PauseState::editor1: {
        mouse->setLockMouse(false);
        EngineRender.setDebugMode(true);
    }break;
    case PauseState::editor1unpaused: {
        mouse->setLockMouse(false);
        EngineRender.setDebugMode(true);
    }break;
    case PauseState::editor2: {
        mouse->setLockMouse(false);
        Engine.get().getParticles().particles_enabled = true;
    }break;
    }

    _currentstate = pause;
}

void CModuleGameManager::update(float delta) {

    updateGameCondition();

    {
        // Escape button
        if (EngineInput["btPause"].getsPressed() && _currentstate != PauseState::defeat) {
            setPauseState(PauseState::main);
        }

        // Lost focus
        if (CApp::get().lostFocus) {
            CApp::get().lostFocus = false;  
            if (_currentstate != PauseState::defeat) {
                setPauseState(PauseState::main);
            }
        }

        // F1 button, flyover, special case
        if (EngineInput["btDebugPause"].getsPressed()) {

            CEntity * e_camera = _fly_camera;
            TCompCameraFlyover * flyover = e_camera->get<TCompCameraFlyover>();
            flyover->setStatus(!flyover->paused);

            CEntity * e_player = _player;
            TMsgScenePaused msg;
            msg.isPaused = (_currentstate == PauseState::none && flyover->paused) ? false : true;
            e_player->sendMsg(msg);
        }

        // F3 button, inspector
        if (EngineInput["btDebugMode"].getsPressed()) {
            setPauseState(PauseState::editor1);
        }

        // F4 button, particles
        if (EngineInput["btDebugParticles"].getsPressed()) {
            setPauseState(PauseState::editor2);
        }

        // F5 button, inspector unpaused
        if (EngineInput["btDebugModeUnpaused"].getsPressed()) {
            setPauseState(PauseState::editor1unpaused);
        }
    }

    {
        if (EngineInput["btUpAux"].getsPressed()) {
            menuPosition = (menuPosition - 1) % menuSize;
        }

        if (EngineInput["btDownAux"].getsPressed()) {
            menuPosition = (menuPosition + 1) % menuSize;
        }
    }
}

void CModuleGameManager::updateGameCondition() {

    _player = EngineEntities.getPlayerHandle();
    _fly_camera = getEntityByName("test_camera_flyover");
    auto& handles = CTagsManager::get().getAllEntitiesByTag(getID("victory_trigger"));

    for (unsigned int i = 0; i < handles.size() && _currentstate != PauseState::win; i++) {

        CEntity* eCollider = handles[i];
        TCompCollider * e = eCollider->get<TCompCollider>();
        if (e && e->player_inside  && _currentstate != PauseState::win) {
            setPauseState(PauseState::win);
        }
    }

    if (_player.isValid() && _currentstate != PauseState::defeat) {

        CEntity* e = _player;
        TCompTempPlayerController *playerCont = e->get<TCompTempPlayerController>();
        if (playerCont->isDead()) {
            Engine.getGUI().getVariables().setVariant("lifeBarFactor", 0);
            setPauseState(PauseState::defeat);
        }
    }
}

void CModuleGameManager::renderMain() {

    VEC2 menu_position = VEC2(CApp::get().xres * .5f - (window_width * .5f),
         CApp::get().yres * .5f - (window_height * .5f));

    // Replace this with separated menus
    if (_currentstate == PauseState::main) {

        ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height));
        ImGui::Begin("MENU", false, window_flags);
        ImGui::CaptureMouseFromApp(false);
        ImGui::SetWindowPos("MENU", ImVec2(menu_position.x, menu_position.y));

        ImGui::Selectable("Resume game", menuPosition == 0);
        if (ImGui::IsItemClicked() || (menuPosition == 0 && EngineInput["btMenuConfirm"].getsPressed()))
        {
            setPauseState(PauseState::none);
        }

        ImGui::Selectable("Restart from last checkpoint", menuPosition == 1);
        if (ImGui::IsItemClicked() || (menuPosition == 1 && EngineInput["btMenuConfirm"].getsPressed()))
        {
            resetState();
            CEntity* e = EngineEntities.getPlayerHandle();
            if (!e) {
                return;
            }
            TCompTempPlayerController* playerCont = e->get<TCompTempPlayerController>();
            //if (playerCont->isDead())
            setPauseState(PauseState::none);

            EngineScene.loadScene(EngineScene.getActiveScene()->name);
        }

        ImGui::Selectable("Restart level", menuPosition == 2);
        if (ImGui::IsItemClicked() || (menuPosition == 2 && EngineInput["btMenuConfirm"].getsPressed()))
        {
            resetState();
            CEntity* e = EngineEntities.getPlayerHandle();
            if (!e) {
                return;
            }
            TCompTempPlayerController* playerCont = e->get<TCompTempPlayerController>();
            //if (playerCont->isDead())
            setPauseState(PauseState::none);

            lastCheckpoint->deleteCheckPoint();
            EngineScene.loadScene(EngineScene.getActiveScene()->name);
        }

        ImGui::Selectable("Exit game", menuPosition == 3);
        if (ImGui::IsItemClicked() || (menuPosition == 3 && EngineInput["btMenuConfirm"].getsPressed()))
        {
            exit(0);
        }

        ImGui::End();
    }
    else if (_currentstate == PauseState::win) {

        ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height));
        ImGui::Begin("VICTORY!", false, window_flags);
        ImGui::CaptureMouseFromApp(false);
        ImGui::SetWindowPos("VICTORY!", ImVec2(menu_position.x, menu_position.y));
        ImGui::Text("Enjoy your dopamine shot");
        ImGui::End();

    }
    else if (_currentstate == PauseState::defeat) {

        ImGui::SetNextWindowSize(ImVec2((float)window_width * 1.2f, (float)window_height));
        ImGui::Begin("YOU DIED! WHAT WOULD YOU DO?", false, window_flags);
        ImGui::CaptureMouseFromApp(false);
        ImGui::SetWindowPos("YOU DIED! WHAT WOULD YOU DO?", ImVec2(menu_position.x, menu_position.y));

        ImGui::Selectable("Restart from last checkpoint", menuPosition == 1);
        if (ImGui::IsItemClicked() || (menuPosition == 1 && EngineInput["btMenuConfirm"].getsPressed()))
        {
            CEntity* e = EngineEntities.getPlayerHandle();
            if (!e) {
                return;
            }
            TCompTempPlayerController* playerCont = e->get<TCompTempPlayerController>();
            //if (playerCont->isDead())
            setPauseState(PauseState::none);

            EngineScene.loadScene(EngineScene.getActiveScene()->name);
        }

        ImGui::Selectable("Restart level", menuPosition == 2);
        if (ImGui::IsItemClicked() || (menuPosition == 2 && EngineInput["btMenuConfirm"].getsPressed()))
        {
            CEntity* e = EngineEntities.getPlayerHandle();
            if (!e) {
                return;
            }
            TCompTempPlayerController* playerCont = e->get<TCompTempPlayerController>();
            //if (playerCont->isDead())
            setPauseState(PauseState::none);

            lastCheckpoint->deleteCheckPoint();
            EngineScene.loadScene(EngineScene.getActiveScene()->name);
        }

        ImGui::Selectable("Exit game", menuPosition == 3);
        if (ImGui::IsItemClicked() || (menuPosition == 3 && EngineInput["btMenuConfirm"].getsPressed()))
        {
            ambient.stop();
            exit(0);
        }

        ImGui::End();

    }

    debugRender();
}

bool CModuleGameManager::saveCheckpoint(VEC3 playerPos, QUAT playerRot) {

    return lastCheckpoint->saveCheckPoint(playerPos, playerRot);
}

bool CModuleGameManager::loadCheckpoint() {

    if (lastCheckpoint) {
        return lastCheckpoint->loadCheckPoint();
    }
    else {
        return false;
    }
}

bool CModuleGameManager::deleteCheckpoint() {

    if (lastCheckpoint) {
        return lastCheckpoint->deleteCheckPoint();
    }
    else {
        return false;
    }
}

bool CModuleGameManager::isPaused() const {

    return _currentstate == PauseState::none ? false : true;
}

CModuleGameManager::PauseState CModuleGameManager::getCurrentState() {

    return _currentstate;
}

void CModuleGameManager::resetState() {

    /* Player not dead and game unpaused */
    CApp::get().lostFocus = false;
    EngineRender.setDebugMode(false);
    EngineParticles.particles_enabled = false;

    // Lock/Unlock the cursor
    Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
    mouse->setLockMouse(true);
}

void CModuleGameManager::debugRender() {

    //if (lastCheckpoint)
    //    lastCheckpoint->debugInMenu();

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
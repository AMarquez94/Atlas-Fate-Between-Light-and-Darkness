#pragma once

#include "modules/module_manager.h"
#include "modules/module_render.h"
#include "modules/module_entities.h"
#include "modules/module_ia.h"
#include "modules/system/module_input.h"
#include "modules/system/module_physics.h"
#include "modules/system/module_cameras.h"
#include "modules/system/module_fsm.h"
#include "modules/module_navmeshes.h"
#include "modules/system/module_logic.h"
#include "modules/system/module_sound.h"
#include "modules/system/module_game_console.h"
#include "modules/system/module_gui.h"
#include "modules/system/module_scene_manager.h"
#include "modules/system/module_instancing.h"
#include "modules/system/module_particles.h"
#include "modules/game/module_game_manager.h"
#include "modules/system/module_debug.h"

class CModuleGameManager;

class CEngine
{
public:
    CEngine();
    static CEngine& get();

    bool start();
    bool stop();
    void update(float delta);
    void render();

    // System modules
    CModuleManager& getModules() { return _modules; }
    CModuleRender& getRender() { return _module_render; }
    CModuleIA& getIA() { return _module_ia; }
    CModuleInput& getInput() { return _module_input; }
    CModulePhysics& getPhysics() { return _module_physics; }
    CModuleCameras& getCameras() { return _module_cameras; }
    CModuleEntities& getEntities() { return _module_entities; }
    CModuleNavmesh& getNavmeshes() { return _module_navmesh; }
    CModuleFSM& getFSM() { return _module_fsm; }
    CModuleSound getSound() { return _module_sound; }
    CModuleLogic& getLogic() { return _module_logic; }
    CModuleGameConsole& getGameConsole() { return _module_game_console; }
    CModuleGameConsole getGameConsoleBis() { return _module_game_console; }
    CModuleGUI& getGUI() { return _module_gui; }
    CModuleSceneManager& getSceneManager() { return _module_scene_manager; }
    CModuleInstancing& getInstancing() { return _module_instancing; }
		CModuleParticles& getParticles() { return _module_particles; }

    CModuleDebug& getDebug() { return _module_debug; }

    // Game modules
    CModuleGameManager& getGameManager();

private:
    CModuleManager  _modules;
    CModuleRender   _module_render;
    CModuleEntities _module_entities;
    CModulePhysics _module_physics;
    CModuleIA       _module_ia;
    CModuleInput    _module_input;
    CModuleCameras  _module_cameras;
    CModuleFSM      _module_fsm;
    CModuleSound    _module_sound;
    CModuleNavmesh  _module_navmesh;
    CModuleLogic    _module_logic;
    CModuleGameConsole _module_game_console;
    CModuleGUI _module_gui;
    CModuleSceneManager _module_scene_manager;
    CModuleInstancing _module_instancing;
		CModuleParticles  _module_particles;
    CModuleDebug _module_debug;
};

#define Engine CEngine::get()
#define EngineIA CEngine::get().getIA()
#define EngineInput CEngine::get().getInput()
#define EngineRender CEngine::get().getRender()
#define EngineCameras CEngine::get().getCameras()
#define EnginePhysics CEngine::get().getPhysics()
#define EngineEntities CEngine::get().getEntities()
#define EngineNavmeshes CEngine::get().getNavmeshes()
#define EngineLogic CEngine::get().getLogic()
#define EngineConsole CEngine::get().getGameConsole()
#define EngineSound CEngine::get().getSound()
#define EngineGUI CEngine::get().getGUI()
#define EngineScene CEngine::get().getSceneManager()
#define EngineInstancing CEngine::get().getInstancing()
#define EngineDebug CEngine::get().getDebug()
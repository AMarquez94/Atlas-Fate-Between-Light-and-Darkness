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
#include "modules/system/module_game_console.h"

class CEngine
{
public:
	CEngine();
	static CEngine& get();

	bool start();
	bool stop();
	void update(float delta);
	void render();

	CModuleManager& getModules() { return _modules; }
	CModuleRender& getRender() { return _module_render; }
	CModuleIA& getIA() { return _module_ia; }
	CModuleInput& getInput() { return _module_input; }
	CModulePhysics& getPhysics() { return _module_physics; }
	CModuleCameras& getCameras() { return _module_cameras; }
	CModuleEntities& getEntities() { return _module_entities; }
  CModuleNavmesh& getNavmeshes() { return _module_navmesh; }
	CModuleFSM& getFSM() { return _module_fsm; }
  CModuleLogic& getLogic() { return _module_logic; }
  CModuleGameConsole& getGameConsole() { 
    return _module_game_console; 
  }
  CModuleGameConsole getGameConsoleBis() { return _module_game_console; }

private:
	CModuleManager  _modules;
	CModuleRender   _module_render;
	CModuleEntities _module_entities;
	CModulePhysics _module_physics;
	CModuleIA       _module_ia;
	CModuleInput    _module_input;
	CModuleCameras  _module_cameras;
	CModuleFSM      _module_fsm;
  CModuleNavmesh  _module_navmesh;
  CModuleLogic    _module_logic;
  CModuleGameConsole _module_game_console;
};

#define Engine CEngine::get()
#define EngineInput CEngine::get().getInput()
#define EngineRender CEngine::get().getRender()
#define EngineCameras CEngine::get().getCameras()
#define EnginePhysics CEngine::get().getPhysics()
#define EngineEntities CEngine::get().getEntities()
#define EngineNavmeshes CEngine::get().getNavmeshes()
#define EngineLogic CEngine::get().getLogic()
#define EngineConsole CEngine::get().getGameConsole()
#pragma once

#include "modules/module_manager.h"
#include "modules/module_render.h"
#include "modules/module_entities.h"
#include "modules/module_ia.h"
#include "modules/system/module_input.h"

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

private:
  CModuleManager  _modules;
  CModuleRender   _module_render;
  CModuleEntities _module_entities;
	CModuleIA       _module_ia;
	CModuleInput    _module_input;
};

#define Engine CEngine::get()
#define EngineInput CEngine::get().getInput()

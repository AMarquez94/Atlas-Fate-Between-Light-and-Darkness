#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include <SLB/SLB.hpp>
#include "modules/system/module_game_console.h"

class CModuleLogic : public IModule
{

public:

  /* Enum with event id. Add as many as necessary */
  enum Events {
    GAME_START,
    GAME_END,
    SCENE_START,
    SCENE_END,
    TRIGGER_ENTER,
    TRIGGER_EXIT,
    ENEMY_KILLED,
    PLAYER_ON_SHADOW_ENTER,
    NUM_EVENTS
  };

  std::vector<std::string> log;

  CModuleLogic(const std::string& aname) : IModule(aname) { }

  bool start() override;
  bool stop() override;
  void update(float delta) override;

  bool execScript(const std::string& script);
  bool execEvent(Events event, const std::string& params = "");
  CModuleGameConsole* getConsoleTest();

private:

  SLB::Manager m;
  SLB::Script s{ &m };

  void BootLuaSLB();
  void publishClasses();
  void loadScriptsInFolder(char * path);
};

class auxClass 
{
public:
  auxClass();
  CModuleGameConsole* getConsole();

private:
};
#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include <SLB/SLB.hpp>
#include "modules/system/module_game_console.h"

class CModuleLogic : public IModule
{

public:

  struct ConsoleResult {
    bool success;
    std::string resultMsg;
  };

  struct DelayedScript {
    std::string script;
    float remainingTime;
  };

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
  CModuleLogic* getPointer() { return this; }

  ConsoleResult execScript(const std::string& script);
  bool execScriptDelayed(const std::string& script, float delay);
  bool execEvent(Events event, const std::string& params = "", float delay = 0.f);
  void printLog();

private:

  SLB::Manager* m = new SLB::Manager;
  SLB::Script* s = new SLB::Script(m);

  std::vector<DelayedScript> delayedScripts;

  void BootLuaSLB();
  void publishClasses();
  void loadScriptsInFolder(char * path);
};

/* Auxiliar functions */
CModuleGameConsole* getConsole();
CModuleLogic* getLogic();
void execDelayedScript(const std::string& script, float delay);
void pauseGame(bool pause);
void fpsToggle();
void systemToggle(const std::string&);
void movePlayer(const float, const float, const float);
void blendInCamera(const std::string& cameraName, float blendInTime);
void blendOutCamera(const std::string& cameraName, float blendOutTime);
void staminaInfinite();
void immortal();
void inShadows();
void speedBoost(const float);
void playerInvisible();
void spotlightsToggle();
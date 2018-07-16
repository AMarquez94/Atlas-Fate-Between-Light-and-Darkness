#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include <SLB/SLB.hpp>
#include "modules/system/module_game_console.h"
#include "modules/system/module_particles.h"
#include "input/button.h"
#include "components/player_controller/comp_player_tempcontroller.h"

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

    struct CommandVariable {
        void* variable;
    };

    int spawnedPatrols = 0, spawnedMimetics = 0, spawnedSpotlights = 0;


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
    std::map<int, std::string> _bindings;

    CModuleLogic(const std::string& aname) : IModule(aname) { }

    bool start() override;
    bool stop() override;
    void update(float delta) override;
    CModuleLogic* getPointer() { return this; }

    void execCvar(std::string& script);
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
CModuleParticles* getParticles();
TCompTempPlayerController* getPlayerController();
void execDelayedScript(const std::string& script, float delay);
void pauseEnemies(bool pause);
void deleteEnemies();
void pauseGame(bool pause);
void pausePlayerToggle();
void staminaInfinite();
void immortal();
void inShadows();
void speedBoost(const float speed);
void playerInvisible();
void noClipToggle();
void lanternsDisable(bool disable);
void blendInCamera(const std::string& cameraName, float blendInTime);
void blendOutCamera(const std::string& cameraName, float blendOutTime);
void blendOutActiveCamera(float blendOutTime);
void move(const std::string& entityName, VEC3 pos, VEC3 lookat = VEC3::Zero);
void movePlayer(VEC3);
void spawn(const std::string& type, VEC3 pos, VEC3 lookat = VEC3::Zero);
void bind(const std::string& key, const std::string& script);
void loadScene(const std::string &level);
void unloadScene();
void loadCheckpoint();
void shadowsToggle();
void debugToggle();
void postFXToggle();
void renderNavmeshToggle();
void playSound2D(const std::string& soundName);
void exeShootImpactSound();
void sleep(float time);
void cinematicModeToggle();
void spotlightsToggle();
void animationsToggle();
void wireframeToggle();
void collidersToggle(bool onlyDynamics = false);


/* DEBUG - TODO: Delete */
void sendOrderToDrone(const std::string& droneName, VEC3 position);
void toggle_spotlight(const std::string& lightName);
void toggleButtonCanBePressed(const std::string& buttonName, bool canBePressed);

// Extra cvar commands
void fpsToggle(bool value);
void cg_drawlights(int type);

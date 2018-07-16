#pragma once

#include "modules/module.h"
#include "checkpoints/checkpoint.h"

class CModuleGameManager : public IModule
{
    enum PauseState { default, none, main, win, defeat, editor1, editor2 };
    PauseState _currentstate;

    /* Mantain a handle of the player */
    CHandle _player;
    CHandle _fly_camera;
    CCheckpoint* lastCheckpoint;

    // Menu window related variables.
    ImGuiWindowFlags window_flags;
    unsigned int window_width;
    unsigned int window_height;
    unsigned int menuPosition = 0;
    const unsigned int menuSize = 4;

    void resetState();
    void debugRender();
    void updateGameCondition();
    void setPauseState(PauseState pause);
    void switchState(PauseState pause);

public:

    struct ConfigPublic {
        bool drawfps = true;

    }config;

    CModuleGameManager(const std::string& name) : IModule(name) {}

    bool start() override;
    void update(float delta) override;
    void renderMain();

    bool saveCheckpoint(VEC3 playerPos, QUAT playerRot);
    bool loadCheckpoint();
    bool deleteCheckpoint();
    bool isPaused() const;

    PauseState getCurrentState();
};
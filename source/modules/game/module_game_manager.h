#pragma once

#include "modules/module.h"
#include "checkpoints/checkpoint.h"
#include "sound/soundEvent.h"


class CModuleGameManager : public IModule
{
    enum PauseState;
    PauseState _currentstate;

    /* Mantain a handle of the player */
    CHandle _player;
    CHandle _fly_camera;
    CCheckpoint* lastCheckpoint;

    /* Maintain the sound for ambient sound */
    SoundEvent ambient;

    // Menu window related variables.
    ImGuiWindowFlags window_flags;
    unsigned int window_width;
    unsigned int window_height;
    unsigned int menuPosition = 0;
    const unsigned int menuSize = 4;

    void resetState();
    void debugRender();
    void updateGameCondition();
    
    void switchState(PauseState pause);

public:

    enum PauseState { none, main, win, defeat, editor1, editor1unpaused, editor2, void_state };

    CModuleGameManager(const std::string& name) : IModule(name) {}

	CModuleGameManager* getPointer() { return this; }

    bool start() override;
    void update(float delta) override;
    void renderMain();

	void setPauseState(PauseState pause);
    bool saveCheckpoint(VEC3 playerPos, QUAT playerRot);
    bool loadCheckpoint();
    bool deleteCheckpoint();
    bool isCheckpointSaved() { return lastCheckpoint != nullptr && lastCheckpoint->isSaved(); };
    bool isPaused() const;
	void resetLevel();
	void resetToCheckpoint();
    PauseState getCurrentState();
};
#pragma once

#include "modules/module.h"
#include "checkpoints/checkpoint.h"
#include "sound/soundEvent.h"


class CModuleGameManager : public IModule
{
    enum PauseState;
    PauseState _currentstate;

    enum MusicState;
    MusicState _musicstate;

    /* Mantain a handle of the player */
    CHandle _player;
    CHandle _fly_camera;
    CCheckpoint* lastCheckpoint;

    /* Maintain the sound for ambient sound */
    SoundEvent ambient;
    SoundEvent persecution_theme;
    float persecution_lerp;
    SoundEvent main_theme;
    float main_theme_lerp;
    //SoundEvent transmission;
    SoundEvent finalScene;
    float final_scene_lerp;
    SoundEvent active_voice;

    // Menu window related variables.
    ImGuiWindowFlags window_flags;
    unsigned int window_width;
    unsigned int window_height;
    unsigned int menuPosition;
    const unsigned int menuSize = 4;

    void resetState();
    void debugRender();
    void updateGameCondition();
    void updateMusicState(float dt);

    void changeMusicState(MusicState new_state);
    
    void switchState(PauseState pause);

public:

    enum PauseState { none, main, win, defeat, editor1, editor1unpaused, editor2, void_state };

    enum MusicState { normal, persecution, ending_persecution, player_died, no_music, end_scene, ending_old_ambient, starting_new_ambient };

    CModuleGameManager(const std::string& name) : IModule(name) {}

	CModuleGameManager* getPointer() { return this; }

    bool start() override;
    void update(float delta) override;
    bool stop() override;
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
    bool isCinematicMode;
    void stopAllSoundEvents();
    void playTransmissionSound(bool play);
    void changeToEndScene();
    void preloadFinalSceneSoundEvent();
    void playVoice(const std::string& voice_event);
    void stopVoice();
    void changeMainTheme();
};
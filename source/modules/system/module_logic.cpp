#include "mcv_platform.h"
#include "module_logic.h"
#include "components/comp_tags.h"
#include <experimental/filesystem>
#include "modules/game/module_game_manager.h"
#include "modules/system/module_particles.h"
#include "sound/soundEvent.h"
#include <iostream>
#include "components/lighting/comp_light_dir.h"
#include "components/lighting/comp_light_spot.h"
#include "components/lighting/comp_light_point.h"
#include "components/postfx/comp_render_ao.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/object_controller/comp_button.h"
#include "components/ia/comp_bt_patrol.h"
#include "components/comp_audio.h"
#include "components/ia/comp_bt_player.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/entity_parser.h"
#include "components/camera_controller/comp_camera_thirdperson.h"
#include "components/comp_render.h"
#include "components/comp_particles.h"
#include "windows/app.h"
#include "components/object_controller/comp_noise_emitter.h"
#include "modules/game/module_game_manager.h"
#include "components/postfx/comp_render_blur.h"
#include "components/postfx/comp_render_focus.h"
#include "gui/gui_widget.h"
#include "components/comp_animated_object_controller.h"
#include "components/object_controller/comp_door.h"
#include "components/postfx/comp_render_bloom.h"
#include "input/devices/mouse.h"
#include "components/comp_group.h"

bool CModuleLogic::start() {

    BootLuaSLB();
    execEvent(Events::GAME_START);
    return true;
}

bool CModuleLogic::stop() {

    delete(s);
    delete(m);
    started = false;
    return true;
}

void CModuleLogic::update(float delta) {
    for (int i = delayedSystemScripts.size() - 1; i >= 0 ; i--) {
        delayedSystemScripts[i].remainingTime -= delta;
        if (delayedSystemScripts[i].remainingTime <= 0) {
			std::string aux_call = delayedSystemScripts[i].script; 
            delayedSystemScripts.erase(delayedSystemScripts.begin() + i);
			execScript(aux_call);
        }
    }

    if (!paused) {
		for (int i = delayedScripts.size() - 1; i >= 0; i--) {
            delayedScripts[i].remainingTime -= delta;
            if (delayedScripts[i].remainingTime <= 0) {
				std::string aux_call_delayed = delayedScripts[i].script;      
                delayedScripts.erase(delayedScripts.begin() + i);
				execScript(aux_call_delayed);
            }
        }

        for (auto k : _bindings) {
            Input::TButton button = EngineInput.keyboard().key(k.first);
            if (button.getsPressed()) {
                std::string current_value = k.second;
                execCvar(current_value);
                execScript(current_value);
            }
        }
    }
}

void CModuleLogic::render()
{
    if (ImGui::TreeNode("Logic")) {

        if (ImGui::TreeNode("Delayed Scripts")) {
            for (int i = 0; i < delayedScripts.size(); i++) {
                ImGui::Text("%s - %f", delayedScripts[i].script.c_str(), delayedScripts[i].remainingTime);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("System Delayed Scripts")) {
            for (int i = 0; i < delayedSystemScripts.size(); i++) {
                ImGui::Text("%s - %f", delayedSystemScripts[i].script.c_str(), delayedSystemScripts[i].remainingTime);
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

/* Where we publish all functions that we want and load all the scripts in the scripts folder */
void CModuleLogic::BootLuaSLB() {

    //Publish all the functions
    publishClasses();
    //Load all the scripts
    loadScriptsInFolder("data/scripts");
    //Mark LUA as started
    started = true;
}

/* Load all scripts.lua in given path and its subfolders */
void CModuleLogic::loadScriptsInFolder(char * path)
{
    try {
        if (std::experimental::filesystem::exists(path) && std::experimental::filesystem::is_directory(path)) {

            std::experimental::filesystem::recursive_directory_iterator iter(path);
            std::experimental::filesystem::recursive_directory_iterator end;

            while (iter != end) {
                std::string fileName = iter->path().string();
                if (iter->path().extension().string() == ".lua" &&
                    !std::experimental::filesystem::is_directory(iter->path())) {
                    dbg("File : %s loaded\n", fileName.c_str());
                    s->doFile(fileName);
                }
                std::error_code ec;
                iter.increment(ec);
                if (ec) {
                    fatal("Error while accessing %s: %s\n", iter->path().string().c_str(), ec.message().c_str());
                }
            }
        }
    }
    catch (std::system_error & e) {
        fatal("Exception %s while loading scripts\n", e.what());
    }
}

/* Publish all the classes in LUA */
void CModuleLogic::publishClasses() {

    /* Classes */
    SLB::Class< CModuleGameConsole >("GameConsole", m)
        .comment("This is our wrapper of the console class")
        .set("addCommand", &CModuleGameConsole::addCommandToList);

    SLB::Class< CModuleLogic >("Logic", m)
        .comment("This is our wrapper of the logic class")
        .set("printLog", &CModuleLogic::printLog);

    SLB::Class< CModuleParticles >("Particles", m)
        .comment("This is our wrapper of the particles class")
        .set("killAll", &CModuleParticles::killAll)
        .set("launchDynamicSystemRot", &CModuleParticles::launchDynamicSystemRot)
        .set("launchDynamicSystem", &CModuleParticles::launchDynamicSystem)
        ;

	SLB::Class< CModuleGameManager >("GameManager", m)
		.comment("This is our wrapper of the gamemanager class")
        .property("isCinematicMode", &CModuleGameManager::isCinematicMode)
		.set("resetToCheckpoint", &CModuleGameManager::resetToCheckpoint)
        .set("changeToEndScene", &CModuleGameManager::changeToEndScene) //TODO: Delete
        .set("preloadFinalSceneSoundEvent", &CModuleGameManager::preloadFinalSceneSoundEvent) //TODO: Delete
        .set("changeMainTheme", &CModuleGameManager::changeMainTheme) //TODO: Delete
        ;

    SLB::Class< VEC3 >("VEC3", m)
        .constructor<float, float, float>()
        .comment("This is our wrapper of the VEC3 class")
        .property("x", &VEC3::x)
        .property("y", &VEC3::y)
        .property("z", &VEC3::z);

    SLB::Class< QUAT >("QUAT", m)
        .constructor<float, float, float, float>()
        .comment("This is our wrapper of the QUAT class")
        .property("x", &QUAT::x)
        .property("y", &QUAT::y)
        .property("z", &QUAT::z)
        .property("w", &QUAT::w);

    SLB::Class< TCompTempPlayerController >("PlayerController", m)
        .comment("This is our wrapper of the player controller component")
        .property("inhibited", &TCompTempPlayerController::isInhibited)
        .set("die", &TCompTempPlayerController::die)
        .set("pauseEnemy", &TCompTempPlayerController::pauseEnemy)
        .set("stunEnemy", &TCompTempPlayerController::stunEnemy)
        .set("die", &TCompTempPlayerController::die)
        .set("getLeftWeapon", &TCompTempPlayerController::getLeftWeapon)
        .set("getRightWeapon", &TCompTempPlayerController::getRightWeapon)
        .set("playPlayerStep", &TCompTempPlayerController::playPlayerStep)
        .set("playLandParticles", &TCompTempPlayerController::playLandParticles)
        .set("playSMSpirals", &TCompTempPlayerController::playSMSpirals)
        .set("resetSMFilters", &TCompTempPlayerController::resetSMFilters)
        ;

    SLB::Class< TCompNoiseEmitter >("NoiseEmitter", m)
        .comment("This is our wrapper of the noise emitter component")
        .set("makeNoise", &TCompNoiseEmitter::makeNoise)
        ;

    SLB::Class<TCompLightSpot>("SpotLight", m)
        .comment("This is our wrapper of the spotlight controller")
        .property("isEnabled", &TCompLightSpot::isEnabled);

    SLB::Class<TCompButton>("Button", m)
        .comment("This is our wrapper of the button controller")
        .property("canBePressed", &TCompButton::canBePressed)
        .set("setCanBePressed", &TCompButton::setCanBePressed)
        ;

    SLB::Class < std::vector < CHandle> >("VHandle", m)
        .comment("Testing this")
        .set("size", &std::vector<CHandle>::size)
        //.set("at", &std::vector<CHandle>::at)
        ;

    SLB::Class<TCompAIPatrol>("AIPatrol", m)
        .comment("This is our wrapper of the patrol controller")
        .set("launchInhibitor", &TCompAIPatrol::launchInhibitor)
		.set("stunPlayer", &TCompAIPatrol::stunPlayer)
		.set("attackPlayer", &TCompAIPatrol::attackPlayer)
        .set("playStepParticle", &TCompAIPatrol::playStepParticle)
        .set("shakeCamera", &TCompAIPatrol::shakeCamera)
        .set("playSlamParticle", &TCompAIPatrol::playSlamParticle)
        ;

    SLB::Class<TCompTransform>("Transform", m)
        .comment("This is our wrapper of the transform controller")
        .set("setPosition", &TCompTransform::setPosition)
        .set("getPosition", &TCompTransform::getPosition)
        .set("setRotation", &TCompTransform::setRotation)
        .set("getRotation", &TCompTransform::getRotation)
        .set("lookAt", &TCompTransform::lookAt)
        .set("getDeltaYawToAimTo", &TCompTransform::getDeltaYawToAimTo)
        ;

    SLB::Class<TCompCameraThirdPerson>("TPCamera", m)
        .comment("This is our wrapper of the Third Person Camera")
        .set("resetCamera", &TCompCameraThirdPerson::resetCamera)
        .set("resetCameraTargetPos", &TCompCameraThirdPerson::resetCameraTargetPos);

    SLB::Class <CHandle>("CHandle", m)
        .comment("CHandle wrapper")
        .constructor()
        .set("fromUnsigned", &CHandle::fromUnsigned)
        .set("destroy", &CHandle::destroy)
        .set("isValid", &CHandle::isValid);

    SLB::Class <CEntity>("CEntity", m)
        .comment("CEntity wrapper")
        .set("getCompByName", &CEntity::getCompByName);

    SLB::Class <SoundEvent>("SoundEvent", m)
        .comment("SoundEvent wrapper")
        .set("isValid", &SoundEvent::isValid)
        .set("restart", &SoundEvent::restart)
        .set("stop", &SoundEvent::stop)
        .set("setPaused", &SoundEvent::setPaused)
        .set("setVolume", &SoundEvent::setVolume)
        .set("setPitch", &SoundEvent::setPitch)
        .set("setParameter", &SoundEvent::setParameter)
        .set("getPaused", &SoundEvent::getPaused)
        .set("getVolume", &SoundEvent::getVolume)
        .set("getPitch", &SoundEvent::getPitch)
        .set("getParameter", &SoundEvent::getParameter)
        .set("is3D", &SoundEvent::is3D)
        .set("isRelativeToCameraOnly", &SoundEvent::isRelativeToCameraOnly)
        .set("setIsRelativeToCameraOnly", &SoundEvent::setIsRelativeToCameraOnly)
        .set("isPlaying", &SoundEvent::isPlaying)
        ;

    SLB::Class<TCompAudio>("Audio", m)
        .comment("This is our wrapper of the audio controller")
        .set("playEvent", &TCompAudio::playEvent);

    SLB::Class<TCompRender>("Render", m)
        .comment("This is our wrapper of the render controller")
        .property("visible", &TCompRender::visible);

    SLB::Class<TCompParticles>("Particles", m)
        .comment("This is our wrapper of the particle controller")
        .set("setSystemState", &TCompParticles::setSystemState);

    SLB::Class<TCompAnimatedObjController>("AnimatedObject", m)
        .comment("This is our wrapper of the animated object controller")
        .set("playAnimation", &TCompAnimatedObjController::playAnimation)
        ;

    SLB::Class<TCompDoor>("Door", m)
        .comment("This is our wrapper of the door controller")
        .set("open", &TCompDoor::open)
        .set("close", &TCompDoor::close)
        .set("setOpenedScript", &TCompDoor::setOpenedScript)
        .set("setClosedScript", &TCompDoor::setClosedScript)
        ;

    /* Global functions */

    // Utilities
    m->set("getConsole", SLB::FuncCall::create(&getConsole));
    m->set("getLogic", SLB::FuncCall::create(&getLogic));
	m->set("getGameManager", SLB::FuncCall::create(&getGameManager));
    m->set("getParticles", SLB::FuncCall::create(&getParticles));
    m->set("getPlayerController", SLB::FuncCall::create(&getPlayerController));
    m->set("getPlayerNoiseEmitter", SLB::FuncCall::create(&getPlayerNoiseEmitter));
    m->set("execScriptDelayed", SLB::FuncCall::create(&execDelayedScript));
    m->set("execSystemScriptDelayed", SLB::FuncCall::create(&execDelayedSystemScript));
    m->set("pauseGame", SLB::FuncCall::create(&pauseGame));
    m->set("pauseEnemies", SLB::FuncCall::create(&pauseEnemies));
    m->set("pauseEnemyEntities", SLB::FuncCall::create(&pauseEnemyEntities));
    m->set("deleteEnemies", SLB::FuncCall::create(&deleteEnemies));
    m->set("isDebug", SLB::FuncCall::create(&isDebug));
    m->set("changeGamestate", SLB::FuncCall::create(&changeGamestate));

    // Camera
    m->set("blendInCamera", SLB::FuncCall::create(&blendInCamera));
    m->set("blendOutCamera", SLB::FuncCall::create(&blendOutCamera));
    m->set("blendOutActiveCamera", SLB::FuncCall::create(&blendOutActiveCamera));
    m->set("resetMainCameras", SLB::FuncCall::create(&resetMainCameras));

    // Player hacks
    m->set("pausePlayerToggle", SLB::FuncCall::create(&pausePlayerToggle));
    m->set("infiniteStamineToggle", SLB::FuncCall::create(&infiniteStamineToggle));
    m->set("immortal", SLB::FuncCall::create(&immortal));
    m->set("inShadows", SLB::FuncCall::create(&inShadows));
    m->set("speedBoost", SLB::FuncCall::create(&speedBoost));
    m->set("playerInvisible", SLB::FuncCall::create(&playerInvisible));
    m->set("noClipToggle", SLB::FuncCall::create(&noClipToggle));

    // postfx hacks
    m->set("postFXToggle", SLB::FuncCall::create(&postFXToggle));
    // sounds
    m->set("playEvent", SLB::FuncCall::create(&playEvent));
    m->set("stopAllAudioComponents", SLB::FuncCall::create(&stopAllAudioComponents));

    // tutorial
    m->set("setTutorialPlayerState", SLB::FuncCall::create(&setTutorialPlayerState));

    // cinematic
    m->set("setCinematicPlayerState", SLB::FuncCall::create(&setCinematicPlayerState));
	m->set("setAIState", SLB::FuncCall::create(&setAIState));
	m->set("speedUpRuedasFinalScene", SLB::FuncCall::create(&speedUpRuedasFinalScene));
	m->set("stopRuedasFinalScene", SLB::FuncCall::create(&stopRuedasFinalScene));
	m->set("lightUpForFinalScene", SLB::FuncCall::create(&lightUpForFinalScene));
	m->set("lightDownForFinalScene", SLB::FuncCall::create(&lightDownForFinalScene));
	m->set("pasarelaLightsFadeOut", SLB::FuncCall::create(&pasarelaLightsFadeOut));
	m->set("ambientAdjustmentForFinalScene", SLB::FuncCall::create(&ambientAdjustmentForFinalScene));
	
	//GUI
	m->set("unPauseGame", SLB::FuncCall::create(&unPauseGame));
	m->set("backFromControls", SLB::FuncCall::create(&backFromControls));
	m->set("unlockDeadButton", SLB::FuncCall::create(&unlockDeadButton));
	m->set("execDeadButton", SLB::FuncCall::create(&execDeadButton));
	m->set("takeOutBlackScreen", SLB::FuncCall::create(&takeOutBlackScreen));
	m->set("goToMainMenu", SLB::FuncCall::create(&goToMainMenu));
	m->set("takeOutCredits", SLB::FuncCall::create(&takeOutCredits));
	m->set("takeOutControlsOnMainMenu", SLB::FuncCall::create(&takeOutControlsOnMainMenu));
	m->set("takeOutCreditsOnMainMenu", SLB::FuncCall::create(&takeOutCreditsOnMainMenu));
	m->set("activateSubtitles", SLB::FuncCall::create(&activateSubtitles));
	m->set("deactivateSubtitles", SLB::FuncCall::create(&deactivateSubtitles));
	m->set("activateMission", SLB::FuncCall::create(&activateMission));
	m->set("setEnemyHudState", SLB::FuncCall::create(&setEnemyHudState));
	m->set("activateCinematicVideoIntro", SLB::FuncCall::create(&activateCinematicVideoIntro));
	m->set("deactivateCinematicVideoIntro", SLB::FuncCall::create(&deactivateCinematicVideoIntro));
	m->set("setInBlackScreen", SLB::FuncCall::create(&setInBlackScreen));
	m->set("setOutBlackScreen", SLB::FuncCall::create(&setOutBlackScreen));
	m->set("subClear", SLB::FuncCall::create(&subClear));
	m->set("playVoice", SLB::FuncCall::create(&playVoice));
	m->set("execLastAtlasScreen", SLB::FuncCall::create(&execLastAtlasScreen));
	m->set("removeAtlasSplash", SLB::FuncCall::create(&removeAtlasSplash));
	m->set("removeTempCredits", SLB::FuncCall::create(&removeTempCredits));
	m->set("activateCredits", SLB::FuncCall::create(&activateCredits));
	
    // Other
    m->set("lanternsDisable", SLB::FuncCall::create(&lanternsDisable));
    m->set("shadowsToggle", SLB::FuncCall::create(&shadowsToggle));
    m->set("debugToggle", SLB::FuncCall::create(&debugToggle));
    m->set("spawn", SLB::FuncCall::create(&spawn));
    m->set("move", SLB::FuncCall::create(&move));
    m->set("bind", SLB::FuncCall::create(&bind));
    m->set("loadCheckpoint", SLB::FuncCall::create(&loadCheckpoint));
    m->set("loadScene", SLB::FuncCall::create(&loadscene));
    m->set("unloadScene", SLB::FuncCall::create(&unloadscene));
    m->set("preloadScene", SLB::FuncCall::create(&preloadScene));
    m->set("cg_drawfps", SLB::FuncCall::create(&cg_drawfps));
    m->set("cg_drawlights", SLB::FuncCall::create(&cg_drawlights));
    m->set("renderNavmeshToggle", SLB::FuncCall::create(&renderNavmeshToggle));
    m->set("sleep", SLB::FuncCall::create(&sleep));
    m->set("cinematicModeToggle", SLB::FuncCall::create(&cinematicModeToggle));
    m->set("isCheckpointSaved", SLB::FuncCall::create(&isCheckpointSaved));
    m->set("destroyHandle", SLB::FuncCall::create(&destroyHandle));
    m->set("destroyHandleByName", SLB::FuncCall::create(&destroyHandleByName));
    m->set("destroyHandleByTag", SLB::FuncCall::create(&destroyHandleByTag));
    m->set("resetPatrolLights", SLB::FuncCall::create(&resetPatrolLights));
    m->set("animateSoundGraph", SLB::FuncCall::create(&animateSoundGraph));
    m->set("makeVisibleByTag", SLB::FuncCall::create(&makeVisibleByTag));
    m->set("getPlayerLocalCoordinatesInReferenceTo", SLB::FuncCall::create(&getPlayerLocalCoordinatesInReferenceTo));
    m->set("getEntityLocalCoordinatesInReferenceTo", SLB::FuncCall::create(&getEntityLocalCoordinatesInReferenceTo));
    m->set("movePlayerToRefPos", SLB::FuncCall::create(&movePlayerToRefPos));
    m->set("moveEntityToRefPos", SLB::FuncCall::create(&moveEntityToRefPos));
    m->set("invalidatePlayerPhysxCache", SLB::FuncCall::create(&invalidatePlayerPhysxCache));
    m->set("GUI_EnableRemoveInhibitor", SLB::FuncCall::create(&GUI_EnableRemoveInhibitor));
    m->set("sendPlayerIlluminatedMsg", SLB::FuncCall::create(&sendPlayerIlluminatedMsg));
    m->set("isInCinematicMode", SLB::FuncCall::create(&isInCinematicMode));
    m->set("preloadSoundEvent", SLB::FuncCall::create(&preloadSoundEvent));
    m->set("stopRenderingEntities", SLB::FuncCall::create(&stopRenderingEntities));
    m->set("changeMainTheme", SLB::FuncCall::create(&changeMainTheme));

    /* Only for debug */
    m->set("sendOrderToDrone", SLB::FuncCall::create(&sendOrderToDrone));
    m->set("toggle_spotlight", SLB::FuncCall::create(&toggle_spotlight));
    m->set("toggleButtonCanBePressed", SLB::FuncCall::create(&toggleButtonCanBePressed));
    m->set("getEntityByName", SLB::FuncCall::create(&getEntityByName));
    m->set("removeSceneResources", SLB::FuncCall::create(&removeSceneResources));
    m->set("destroyPartialScene", SLB::FuncCall::create(&destroyPartialScene));
    m->set("testingLoadPartialScene", SLB::FuncCall::create(&testingLoadPartialScene));
    m->set("testLoco", SLB::FuncCall::create(&testLoco));
    m->set("printAllResources", SLB::FuncCall::create(&printAllResources));
    m->set("deleteAllCacheResources", SLB::FuncCall::create(&deleteAllCacheResources));

    /* Handle converters */
    m->set("toEntity", SLB::FuncCall::create(&toEntity));
    m->set("toTransform", SLB::FuncCall::create(&toTransform));
    m->set("toAIPatrol", SLB::FuncCall::create(&toAIPatrol));
    m->set("toAudio", SLB::FuncCall::create(&toAudio));
    m->set("toTPCamera", SLB::FuncCall::create(&toTPCamera));
    m->set("toRender", SLB::FuncCall::create(&toRender));
    m->set("toParticles", SLB::FuncCall::create(&toParticles));
    m->set("toAnimatedObject", SLB::FuncCall::create(&toAnimatedObject));
    m->set("toDoor", SLB::FuncCall::create(&toDoor));
    m->set("toButton", SLB::FuncCall::create(&toButton));
}

/* Check if it is a fast format command */
void CModuleLogic::execCvar(std::string& script) {

    // Only backslash commands fetched.
    if (script.find("/") != 0)
        return;

    // Little bit of dirty tricks to achieve same results with different string types.
    script.erase(0, 1);
    int index = (int)script.find_first_of(' ');
    script = index != -1 ? script.replace(script.find_first_of(' '), 1, "(") : script;
    index = (int)script.find_first_of(' ');
    script = index != -1 ? script.replace(script.find_first_of(' '), 1, ",") : script;
    script.append(")");
}

CModuleLogic::ConsoleResult CModuleLogic::execScript(const std::string& script) {
    if (!started)
        return ConsoleResult{ false, "" };

    std::string scriptLogged = script;
    std::string errMsg = "";
    bool success = false;

    try {
        s->doString(script);
        scriptLogged = scriptLogged + " - Success";
        success = true;
    }
    catch (std::exception e) {
        scriptLogged = scriptLogged + " - Failed";
        scriptLogged = scriptLogged + "\n" + e.what();
        errMsg = e.what();
    }

    dbg("Script %s\n", scriptLogged.c_str());
    log.push_back(scriptLogged);
    return ConsoleResult{ success, errMsg };
}

bool CModuleLogic::execScriptDelayed(const std::string & script, float delay)
{
    delayedScripts.push_back(DelayedScript{ script, delay });
    return true;
}

bool CModuleLogic::execSystemScriptDelayed(const std::string & script, float delay)
{
    delayedSystemScripts.push_back(DelayedScript{ script, delay });
    return true;
}

bool CModuleLogic::execEvent(Events event, const std::string & params, float delay)
{
    /* TODO: meter eventos */
    switch (event) {
    case Events::GAME_START:
        if (delay > 0) {
            return execScriptDelayed("onGameStart()", delay);
        }
        else {
            return execScript("onGameStart()").success;
        }
        break;
    case Events::GAME_END:
        if (delay > 0) {
            return execScriptDelayed("onGameEnd()", delay);
        }
        else {
            return execScript("onGameEnd()").success;
        }
        break;
    case Events::SCENE_START:
        if (delay > 0) {
            return execScriptDelayed("onSceneStart()", delay) && execScriptDelayed("onSceneStart_" + params + "()", delay);
        }
        else {
            return execScript("onSceneStart()").success && execScript("onSceneStart_" + params + "()").success;
        }
        break;
    case Events::SCENE_END:
        if (delay > 0) {
            return execScriptDelayed("onSceneEnd()", delay) && execScriptDelayed("onSceneEnd_" + params + "()", delay);
        }
        else {
            return execScript("onSceneEnd()").success && execScript("onSceneEnd_" + params + "()").success;
        }
        break;
    case Events::SCENE_PARTIAL_START:
        if (delay > 0) {
            return execScriptDelayed("onSceneStart()", delay) && execScriptDelayed("onScenePartialStart_" + params + "()", delay);
        }
        else {
            return execScript("onSceneStart()").success && execScript("onScenePartialStart_" + params + "()").success;
        }
        break;
    case Events::SCENE_PARTIAL_END:
        if (delay > 0) {
            return execScriptDelayed("onScenePartialEnd()", delay) && execScriptDelayed("onScenePartialEnd_" + params + "()", delay);
        }
        else {
            return execScript("onScenePartialEnd()").success && execScript("onScenePartialEnd_" + params + "()").success;
        }
        break;

    case Events::TRIGGER_ENTER:
        if (delay > 0) {
            return execScriptDelayed("onTriggerEnter_" + params, delay);
        }
        else {
            return execScript("onTriggerEnter_" + params).success;
        }
        break;
    case Events::TRIGGER_EXIT:
        if (delay > 0) {
            return execScriptDelayed("onTriggerExit_" + params, delay);
        }
        else {
            return execScript("onTriggerExit_" + params).success;
        }
        break;
    case Events::PATROL_STUNNED:
        if (delay > 0) {
            return execScriptDelayed("onPatrolStunned_" + params, delay);
        }
        else {
            return execScript("onPatrolStunned_" + params).success;
        }
        break;
    case Events::PATROL_KILLED:
        if (delay > 0) {
            return execScriptDelayed("onPatrolKilled_" + params, delay);
        }
        else {
            return execScript("onPatrolKilled_" + params).success;
        }
        break;
    default:

        break;
    }
    return false;
}

void CModuleLogic::printLog()
{
    dbg("Printing log\n");
    for (int i = 0; i < log.size(); i++) {
        dbg("%s\n", log[i].c_str());
    }
    dbg("End printing log\n");
}

void CModuleLogic::clearDelayedScripts()
{
    delayedScripts.clear();
    //delayedSystemScripts.clear();
}

void CModuleLogic::eraseDelayedScripts(std::string keyWord) {
	
	for (int i = delayedScripts.size() - 1; i >= 0 ; i--) {
		CModuleLogic::DelayedScript _curr_del = delayedScripts[i];
		std::string::size_type e = _curr_del.script.find(keyWord);
		if ((int)e >-1) {
			delayedScripts.erase(delayedScripts.begin() + i);
		}	
	}
}

void CModuleLogic::eraseSystemDelayedScripts(std::string keyWord) {

	for (int i = delayedSystemScripts.size() - 1; i >= 0; i--) {
		CModuleLogic::DelayedScript _curr_del = delayedSystemScripts[i];
		std::string::size_type e = _curr_del.script.find(keyWord);
		if ((int)e >-1) {
			delayedSystemScripts.erase(delayedSystemScripts.begin() + i);
		}
	}
}

/* Auxiliar functions */
CModuleLogic * getLogic() { return EngineLogic.getPointer(); }

CModuleGameManager * getGameManager() { return CEngine::get().getGameManager().getPointer(); }

CModuleParticles * getParticles() { return EngineParticles.getPointer(); }

TCompTempPlayerController * getPlayerController()
{
    TCompTempPlayerController * playerController = nullptr;
    CEntity* e = EngineEntities.getPlayerHandle();
    if (e) {
        playerController = e->get<TCompTempPlayerController>();
    }
    return playerController;
}

TCompNoiseEmitter * getPlayerNoiseEmitter()
{
    TCompNoiseEmitter * playerNoiseEmitter = nullptr;
    CEntity* e = EngineEntities.getPlayerHandle();
    if (e) {
        playerNoiseEmitter = e->get<TCompNoiseEmitter>();
    }
    return playerNoiseEmitter;
}

CModuleGameConsole * getConsole() { return EngineConsole.getPointer(); }

void execDelayedScript(const std::string& script, float delay)
{
    EngineLogic.execScriptDelayed(script, delay);
}

void execDelayedSystemScript(const std::string & script, float delay)
{
    EngineLogic.execSystemScriptDelayed(script, delay);
}

void pauseEnemies(bool pause) {

    std::vector<CHandle> enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
    TMsgAIPaused msg;
    msg.isPaused = pause;
    for (int i = 0; i < enemies.size(); i++) {
        enemies[i].sendMsg(msg);
    }
}

void pauseEnemyEntities(bool pause) {
    std::vector<CHandle> enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
    TMsgScenePaused msg;
    msg.isPaused = pause;
    for (int i = 0; i < enemies.size(); i++) {
        enemies[i].sendMsg(msg);
    }
}

void deleteEnemies()
{
    VHandles enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
    for (auto h : enemies) {
        h.destroy();
    }
}

bool isDebug()
{
    #ifdef NDEBUG
        return false;
    #else
        return true;
    #endif
}

void changeGamestate(const std::string& gamestate) {
    EngineScene.changeGameState(gamestate);
}

void pauseGame(bool pause) {

    TMsgScenePaused msg;
    msg.isPaused = pause;
    EngineEntities.broadcastMsg(msg);
}

void infiniteStamineToggle() {
    TMsgInfiniteStamina msg;
    CHandle h = EngineEntities.getPlayerHandle();
    h.sendMsg(msg);
}

void immortal(bool state) {
    CHandle h = EngineEntities.getPlayerHandle();
    TMsgPlayerImmortal msg;
	msg.state = state;
    h.sendMsg(msg);
}

void inShadows() {
    CHandle h = EngineEntities.getPlayerHandle();
    TMsgPlayerInShadows msg;
    h.sendMsg(msg);
}

void speedBoost(const float speed) {
    CHandle h = EngineEntities.getPlayerHandle();
    TMsgSpeedBoost msg;
    msg.speedBoost = speed;
    h.sendMsg(msg);
}

void playerInvisible() {
    CHandle h = EngineEntities.getPlayerHandle();
    TMsgPlayerInvisible msg;
    h.sendMsg(msg);
}

void noClipToggle()
{
    CHandle h = EngineEntities.getPlayerHandle();
    TMsgSystemNoClipToggle msg;
    h.sendMsg(msg);
}

void lanternsDisable(bool disable) {
    VHandles patrols = CTagsManager::get().getAllEntitiesByTag(getID("patrol"));
    TMsgLanternsDisable msg{ disable };
    for (auto h : patrols) {
        h.sendMsg(msg);
    }
}

void blendInCamera(const std::string & cameraName, float blendInTime, const std::string& mode, const std::string& interpolator) {

    CHandle camera = getEntityByName(cameraName);
    if (camera.isValid()) {
        EngineCameras.blendInCamera(camera, blendInTime, EngineCameras.getPriorityFromString(mode), EngineCameras.getInterpolatorFromString(interpolator));
    }
    //TODO: implement
}

void blendOutCamera(const std::string & cameraName, float blendOutTime) {

    CHandle camera = getEntityByName(cameraName);
    if (camera.isValid()) {
        EngineCameras.blendOutCamera(camera, blendOutTime);
    }
}

void blendOutActiveCamera(float blendOutTime) {
    EngineCameras.blendOutCamera(EngineCameras.getCurrentCamera(), blendOutTime);
}

void resetMainCameras()
{
    std::vector<CHandle> v_cameras = CTagsManager::get().getAllEntitiesByTag(getID("main_camera"));
    for (int i = 0; i < v_cameras.size(); i++) {
        TMsgCameraResetTargetPos msg;
        v_cameras[i].sendMsg(msg);
    }
}

/* Spawn item on given position */
CHandle spawn(const std::string & name, const VEC3 & pos, const VEC3& lookat) {
    TEntityParseContext ctxSpawn;
    parseScene("data/prefabs/" + name + ".prefab", ctxSpawn);
    CHandle h = ctxSpawn.entities_loaded[0];
    CEntity* e = h;
    TCompTransform* e_pos = e->get<TCompTransform>();
    e_pos->lookAt(pos, lookat);
    return h;
}

void move(const std::string & name, const VEC3 & pos, const VEC3 & lookat)
{
    CHandle h_to_move = getEntityByName(name);
    if (h_to_move.isValid()) {
        CEntity* e_to_move = h_to_move;
        TCompTransform* e_pos = e_to_move->get<TCompTransform>();
        e_pos->lookAt(pos, lookat);
        TCompCollider* e_collider = e_to_move->get<TCompCollider>();
        TCompRigidbody* e_rigidbody = e_to_move->get<TCompRigidbody>();
        if (e_rigidbody) {
            e_rigidbody->setGlobalPose(pos, e_pos->getRotation());
        }
        else if (e_collider) {
            e_collider->setGlobalPose(pos, e_pos->getRotation());
        }
    }
}

void loadscene(const std::string &level) {
    EngineScene.loadScene(level);
}

void unloadscene() {
    EngineScene.unLoadActiveScene();
}

void preloadScene(const std::string & scene)
{
    EngineScene.preloadScene(scene);
}

void sleep(float time) {
    Sleep(time);
}

void cinematicModeToggle() {
    TMsgPlayerAIEnabled msg;
    msg.state = "cinematic";
    msg.enableAI = true;
    CHandle h = EngineEntities.getPlayerHandle();
    h.sendMsg(msg);
}

bool isCheckpointSaved()
{
    CModuleGameManager gameManager = CEngine::get().getGameManager();
    return gameManager.isCheckpointSaved();
}

void destroyHandle(unsigned int h)
{
    CHandle handle;
    handle.fromUnsigned(h);
    handle.destroy();
}

void destroyHandleByName(const std::string & name)
{
    if (name != "") {
        CHandle h = getEntityByName(name);
        if (h.isValid()) {
            h.destroy();
        }
    }
}

void destroyHandleByTag(const std::string& tag) {
    if (tag != "") {
        VHandles v = CTagsManager::get().getAllEntitiesByTag(getID(tag.c_str()));
        for (int i = 0; i < v.size(); i++) {
            v[i].destroy();
        }
    }
}

void resetPatrolLights()
{
    VHandles patrols = CTagsManager::get().getAllEntitiesByTag(getID("patrol"));
    TMsgResetPatrolLights msg;
    for (int i = 0; i < patrols.size(); i++) {
        patrols[i].sendMsg(msg);
    }
}

void animateSoundGraph(int value) {
	EngineGUI.getWidget(CModuleGUI::EGUIWidgets::SOUND_GRAPH)->getChild("sound_sprite")->getSpriteParams()->_playing_sprite = value;
}

void makeVisibleByTag(const std::string & tag, bool visible)
{
    TMsgSetVisible msg;
    msg.visible = visible;
    VHandles v_handles = CTagsManager::get().getAllEntitiesByTag(getID(tag.c_str()));
    for (int i = 0; i < v_handles.size(); i++) {
        v_handles[i].sendMsg(msg);
    }
}

VEC3 getPlayerLocalCoordinatesInReferenceTo(const std::string & ref_entity)
{
    CEntity* player = EngineEntities.getPlayerHandle();
    CEntity* e_ref_entity = getEntityByName(ref_entity);
    TCompTransform* ppos = player->get<TCompTransform>();
    TCompTransform* ref_pos = e_ref_entity->get<TCompTransform>();

    MAT44 ref_trans = ref_pos->asMatrix().Invert();
    return VEC3::Transform(ppos->getPosition(), ref_trans);
}

VEC3 getEntityLocalCoordinatesInReferenceTo(const std::string & entityToGet, const std::string & ref_entity)
{
    CEntity* e_entity_to_get = getEntityByName(entityToGet);
    CEntity* e_ref_entity = getEntityByName(ref_entity);
    if (e_entity_to_get && e_ref_entity) {
        TCompTransform* epos = e_entity_to_get->get<TCompTransform>();
        TCompTransform* ref_pos = e_ref_entity->get<TCompTransform>();

        MAT44 ref_trans = ref_pos->asMatrix().Invert();
        return VEC3::Transform(epos->getPosition(), ref_trans);
    }
    else {
        return VEC3::Zero;
    }
   
}

void movePlayerToRefPos(const std::string & ref_entity, VEC3 p_rel_pos)
{
    CEntity* player = EngineEntities.getPlayerHandle();
    TCompTransform* tplayer = player->get<TCompTransform>();
    CEntity* final_reference = getEntityByName(ref_entity);
    TCompTransform* t_final_reference = final_reference->get<TCompTransform>();

    /* Rotate the vector */
    QUAT rot_final = t_final_reference->getRotation();
    VEC3 u = VEC3(rot_final.x, rot_final.y, rot_final.z);
    float s = rot_final.w;
    p_rel_pos = 2.0f * u.Dot(p_rel_pos) * u
        + (s*s - u.Dot(u)) * p_rel_pos
        + 2.0f * s * u.Cross(p_rel_pos);

    VEC3 final_pos = p_rel_pos + t_final_reference->getPosition();
    tplayer->setPosition(final_pos);
    TCompRigidbody* my_rigidbody = player->get<TCompRigidbody>();
    my_rigidbody->setGlobalPose(tplayer->getPosition(), tplayer->getRotation());
}

void moveEntityToRefPos(const std::string & entity_to_move, const std::string & ref_entity, VEC3 p_rel_pos)
{
    CEntity* e_entity_to_move = getEntityByName(entity_to_move);
    TCompTransform* t_entity_to_move = e_entity_to_move->get<TCompTransform>();
    CEntity* final_reference = getEntityByName(ref_entity);
    TCompTransform* t_final_reference = final_reference->get<TCompTransform>();

    /* Rotate the vector */
    QUAT rot_final = t_final_reference->getRotation();
    VEC3 u = VEC3(rot_final.x, rot_final.y, rot_final.z);
    float s = rot_final.w;
    p_rel_pos = 2.0f * u.Dot(p_rel_pos) * u
        + (s*s - u.Dot(u)) * p_rel_pos
        + 2.0f * s * u.Cross(p_rel_pos);

    VEC3 final_pos = p_rel_pos + t_final_reference->getPosition();
    t_entity_to_move->setPosition(final_pos);
}

void invalidatePlayerPhysxCache() {
    CEntity* ePlayer = EngineEntities.getPlayerHandle();
    TCompRigidbody* tRigidbody = ePlayer->get <TCompRigidbody>();
    tRigidbody->invalidateCache();
}

void GUI_EnableRemoveInhibitor() {

    CEntity* ePlayer = EngineEntities.getPlayerHandle();
    TCompTempPlayerController* pController = ePlayer->get<TCompTempPlayerController>();
    if (!EngineInput.pad().connected) {
        EngineGUI.enableWidget("inhibited_space", pController->isInhibited);
    }
    else {
        EngineGUI.enableWidget("inhibited_y", pController->isInhibited);
    }
}

void sendPlayerIlluminatedMsg(CHandle h, bool illuminated) {
    if (h.isValid()) {
        CHandle player = EngineEntities.getPlayerHandle();
        TMsgPlayerIlluminated msg;
        msg.h_sender = h;
        msg.isIlluminated = illuminated;
        player.sendMsg(msg);
    }
}

void isInCinematicMode(bool isCinematic)
{
    dbg("SETEAMOS A %s\n", isCinematic ? "TRUE" : "FALSE");
    CEngine::get().getGameManager().isCinematicMode = isCinematic;
    if (isCinematic) {
        // Lock/Unlock the cursor
        Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
        mouse->setOnlyLockMouse(false);
    }
    else {
        // Lock/Unlock the cursor
        Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
        mouse->setOnlyLockMouse(true);
    }
}

SoundEvent preloadSoundEvent(const std::string & soundevent)
{
    return EngineSound.preloadEvent(soundevent);
}

void stopRenderingEntities()
{
    TMsgSetVisible msg;
    msg.visible = false;
    EngineEntities.broadcastMsg(msg);
}

void changeMainTheme()
{
    CEngine::get().getGameManager().changeMainTheme();
}


SoundEvent playEvent(const std::string & name)
{
    return EngineSound.playEvent(name);
}

void stopAllAudioComponents()
{
    TMsgStopAudioComponent msg;
    EngineEntities.broadcastMsg(msg);
}

void setTutorialPlayerState(bool active, const std::string & stateName)
{
    CHandle h_tutorial = getEntityByName("Tutorial Player");    
    TMsgPlayerAIEnabled msg;
    msg.state = stateName;
    msg.enableAI = active;
    h_tutorial.sendMsg(msg);
}

void setCinematicPlayerState(bool active, const std::string & stateName)
{
    CHandle h_tutorial = EngineEntities.getPlayerHandle();
    TMsgPlayerAIEnabled msg;
    msg.state = stateName;
    msg.enableAI = active;
    h_tutorial.sendMsg(msg);
}

void setAIState(const std::string & name, bool active, const std::string & stateName)
{
    CHandle h = getEntityByName(name);
    if (h.isValid()) {
        TMsgCinematicState msg;
        msg.enableCinematic = active;
        msg.state = stateName;
        h.sendMsg(msg);
    }
}

void activateScene(const std::string& scene) {
    //EngineScene.setActiveScene()
}

void loadCheckpoint()
{
    CModuleGameManager gameManager = CEngine::get().getGameManager();
    gameManager.loadCheckpoint();
}

void shadowsToggle()
{
    EngineRender.setGenerateShadows(!EngineRender.getGenerateShadows());
}

void postFXToggle() {
    //Deactivating AO
    EngineRender.setGeneratePostFX(!EngineRender.getGeneratePostFX());

    //Deactivating rest of postFX
    getObjectManager<TCompRenderAO>()->forEach([&](TCompRenderAO* c) {
        c->setState(!c->getState());
    });
}

void pausePlayerToggle() {
    CEntity* p = EngineEntities.getPlayerHandle();
    TCompTempPlayerController* player = p->get<TCompTempPlayerController>();

    TMsgScenePaused stopPlayer;
    stopPlayer.isPaused = !player->paused;
    EngineEntities.broadcastMsg(stopPlayer);
}

void debugToggle()
{
    EngineRender.setDebugMode(!EngineRender.getDebugMode());
}

void bind(const std::string& key, const std::string& script) {

    int id = EngineInput.getButtonDefinition(key)->id;
    //Input::TButton button = EngineInput.keyboard().key(id);
    EngineLogic._bindings[id] = script;
}

void unbind(const std::string& key, const std::string& script) {

    int id = EngineInput.getButtonDefinition(key)->id;
    //Input::TButton button = EngineInput.keyboard().key(id);

    std::map<int, std::string>::iterator it;
    it = EngineLogic._bindings.find(id);
    EngineLogic._bindings.erase(it, EngineLogic._bindings.end());
}

void renderNavmeshToggle() {
    EngineNavmeshes.renderNamvesh = !EngineNavmeshes.renderNamvesh;
}

// Toggle CVARS.
void cg_drawfps(bool value) {
    CApp::get().drawfps = value;
}

void cg_drawlights(int type) {

    bool dir = false, spot = false, point = false;

    switch (type) {
    case 1: dir = spot = point = true; break;
    case 2: dir = true; break;
    case 3: spot = true; break;
    case 4: point = true; break;
    default: break;
    }

    getObjectManager<TCompLightDir>()->forEach([&](TCompLightDir* c) {
        c->isEnabled = dir;
    });

    getObjectManager<TCompLightSpot>()->forEach([&](TCompLightSpot* c) {
        c->isEnabled = spot;
    });

    getObjectManager<TCompLightPoint>()->forEach([&](TCompLightPoint* c) {
        c->isEnabled = point;
    });
}

CEntity* toEntity(CHandle h)
{
    CEntity* e = h;
    return e;
}

TCompTransform* toTransform(CHandle h)
{
    TCompTransform* t = h;
    return t;
}

TCompAIPatrol* toAIPatrol(CHandle h)
{
    TCompAIPatrol* t = h;
    return t;
}

TCompAudio* toAudio(CHandle h)
{
    TCompAudio* t = h;
    return t;
}

TCompParticles* toParticles(CHandle h)
{
    TCompParticles* t = h;
    return t;
}

TCompAnimatedObjController* toAnimatedObject(CHandle h)
{
    TCompAnimatedObjController* t = h;
    return t;
}

TCompDoor* toDoor(CHandle h)
{
    TCompDoor* t = h;
    return t;
}

TCompButton* toButton(CHandle h)
{
    TCompButton* t = h;
    return t;
}

TCompCameraThirdPerson * toTPCamera(CHandle h)
{
    TCompCameraThirdPerson* t = h;
    return t;
}

TCompRender * toRender(CHandle h)
{
    TCompRender* t = h;
    return t;
}

void sendOrderToDrone(const std::string & droneName, VEC3 position)
{
    CEntity* drone = getEntityByName(droneName);
    TMsgOrderReceived msg;
    msg.position = position;
    msg.hOrderSource = EngineEntities.getPlayerHandle();
    drone->sendMsg(msg);
}

void toggle_spotlight(const std::string & lightName)
{
    CHandle hLight = getEntityByName(lightName);
    if (!hLight.isValid()) {
        return;
    }
    CEntity* light = hLight;
    TCompLightSpot* spotlight = light->get<TCompLightSpot>();
    spotlight->isEnabled = !spotlight->isEnabled;
}

void toggleButtonCanBePressed(const std::string & buttonName, bool canBePressed)
{
    CHandle hButton = getEntityByName(buttonName);
    if (!hButton.isValid()) {
        return;
    }
    CEntity* button = hButton;
    TCompButton* comp_button = button->get<TCompButton>();
	if (comp_button) {
		comp_button->canBePressed = canBePressed;
	}
}

void removeSceneResources(const std::string & scene)
{
    EngineScene.removeSceneResources(scene);
}

void destroyPartialScene()
{
    std::vector<uint32_t> tags;
    tags.push_back(getID("corridor"));
    tags.push_back(getID("persistent"));
    VHandles entities_to_destroy = CTagsManager::get().getAllEntitiesWithoutTags(tags);
    for (int i = 0; i < entities_to_destroy.size(); i++) {
        entities_to_destroy[i].destroy();
    }
}

void testingLoadPartialScene() {
    EngineScene.loadPartialScene("scene_coliseo");
}

void testLoco() {

    CEntity* player = EngineEntities.getPlayerHandle();
    TCompTransform* tplayer = player->get<TCompTransform>();
    CEntity* suelo_intro = getEntityByName("intro_suelo001");
    CEntity* suelo_col = getEntityByName("col_intro_suelo001");
    TCompTransform* t_suelo_intro = suelo_intro->get<TCompTransform>();
    TCompTransform* t_suelo_col = suelo_col->get<TCompTransform>();

    MAT44 ref_trans = t_suelo_intro->asMatrix().Invert();
    VEC3 rel_pos = VEC3::Transform(tplayer->getPosition(), ref_trans);

    /* Rotate the vector */
    QUAT rot_final = t_suelo_col->getRotation();
    VEC3 u = VEC3(rot_final.x, rot_final.y, rot_final.z);
    float s = rot_final.w;
    rel_pos = 2.0f * u.Dot(rel_pos) * u
        + (s*s - u.Dot(u)) * rel_pos
        + 2.0f * s * u.Cross(rel_pos);

    VEC3 final_pos = rel_pos + t_suelo_col->getPosition();
    tplayer->setPosition(final_pos);
    TCompRigidbody* my_rigidbody = player->get<TCompRigidbody>();
    my_rigidbody->setGlobalPose(tplayer->getPosition(), tplayer->getRotation());

    /* TODO: rotation */
}

void printAllResources()
{
    Resources.printAllResources();
}

void deleteAllCacheResources()
{
    EngineFiles.deleteAllCacheResources();
}

void unPauseGame() {

	CEngine::get().getGameManager().setPauseState(CModuleGameManager::PauseState::none);
	EngineGUI.setButtonsState(true);
}

void backFromControls() {
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::BACK_BUTTON);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::CONTROLS);
	EngineGUI.activateController(CModuleGUI::EGUIWidgets::INGAME_MENU_PAUSE_BUTTONS);

}

void unlockDeadButton() {
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::DEAD_MENU_BUTTONS)->makeChildsFadeIn(1, 0, true);
}

void execDeadButton() {
	EngineGUI.getWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN)->makeChildsFadeOut(3, 0.3, false);
	EngineGUI.setButtonsState(true);
	Engine.get().getGameManager().resetToCheckpoint();
}

void takeOutBlackScreen() {
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN);
}

void goToMainMenu() {
    EngineScene.changeGameState("main_menu");
}

void takeOutCredits() {
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::INGAME_MENU_PAUSE);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_CREDITS_BACKGROUND);
}

void takeOutControlsOnMainMenu() {
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_CONTROLS_BACKGROUND);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_CONTROLS_BACK);
	EngineGUI.activateController(CModuleGUI::EGUIWidgets::MAIN_MENU_BUTTONS);
}

void takeOutCreditsOnMainMenu() {
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_CREDITS_BACKGROUND);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_CREDITS_BACK);
	EngineGUI.activateController(CModuleGUI::EGUIWidgets::MAIN_MENU_BUTTONS);
}

void activateSubtitles(int sub_num) {

	EngineGUI.setSubtitles(sub_num);
}

void subClear() {
    CEngine::get().getGameManager().stopVoice();
	EngineGUI.clearSubtitles();
}

void playVoice(const std::string & voice_event)
{
    dbg("PLAYING VOICE %s\n", voice_event);
    CEngine::get().getGameManager().playVoice(voice_event);
}

void deactivateSubtitles() {
	EngineGUI.setSubtitlesToNone();
}

void activateMission(int mission_num) {
	EngineGUI.setMission(mission_num);
}

void setEnemyHudState(bool state) {
    CModuleGameManager gameManager = CEngine::get().getGameManager();
	if (state) {
		EngineGUI.activateEnemyHUD();  
        //gameManager.playTransmissionSound(true);
	}
	else {
		EngineGUI.deactivateEnemyHUD();
        //gameManager.playTransmissionSound(false);
    }
}

void activateCinematicVideoIntro(float time_to_lerp, float time_to_start) {
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::CINEMATIC_INTRO)->makeChildsFadeOut(time_to_lerp, time_to_start, false);
}

void deactivateCinematicVideoIntro() {
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::CINEMATIC_INTRO);
}

void setInBlackScreen(float time_to_lerp) {
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN)->makeChildsFadeIn(time_to_lerp,0,false);
}

void setOutBlackScreen(float time_to_lerp) {
	EngineGUI.getWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN)->makeChildsFadeOut(time_to_lerp, 0, false);
	EngineLogic.execScriptDelayed("takeOutBlackScreen();", time_to_lerp + 0.1f);
}

void lightUpForFinalScene(bool random, float time_to_lerp) {
	EngineEntities.broadcastMsg(TMsgEmisiveCapsuleState{ false , random, time_to_lerp});
}

void ambientAdjustmentForFinalScene(float time_to_lerp) {
	EngineLerp.lerpElement(&cb_globals.global_exposure_adjustment,2.2f, time_to_lerp,0.0f);
}

void lightDownForFinalScene(bool random, float time_to_lerp) {
	EngineEntities.broadcastMsg(TMsgEmisiveCapsuleState{ true , random, time_to_lerp});
	EngineLerp.lerpElement(&cb_globals.global_fog_density,0.364f,3,5);
}

void pasarelaLightsFadeOut() {
	EngineEntities.broadcastMsg(TMsgOmniFadeOut{});
}

void speedUpRuedasFinalScene() {
	EngineEntities.broadcastMsg(TMsgRotatorAccelerate{ 10.0f,4.0f,0.0f });

    // Dirty way of working, but no time to place this beautifully somewhere else
    // Shutting down the slow particles.
    CEntity * out = getEntityByName("Particles_Slow");
    TCompGroup * out_group = out->get<TCompGroup>();
    for (auto &p : out_group->handles) {
        CEntity * current = p;
        TCompParticles * part = current->get<TCompParticles>();
        part->setSystemFading(1);
    }

    //Enabling the fast particles
    out = getEntityByName("Particles_Fast");
    out_group = out->get<TCompGroup>();
    for (auto &p : out_group->handles) {
        CEntity * current = p;
        TCompParticles * part = current->get<TCompParticles>();
        part->setSystemState(true);
    }

}

void stopRuedasFinalScene() {
	EngineEntities.broadcastMsg(TMsgRotatorAccelerate{ 0.0f,7.0f,0.0f });

	CEntity* ent = getEntityByName("rueda");
	TCompRender* comp_rend = ent->get<TCompRender>();
	if (comp_rend != nullptr) {
		EngineLerp.lerpElement(&comp_rend->self_intensity,0.0f,7.0f,0.0f);
	}
}

void execLastAtlasScreen() {

	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::ATLAS_LAST_SPLASH)->makeChildsFadeIn(0.1, 0, false);
	
	GUI::CWidget *w = EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::ATLAS_LAST_SPLASH_LINE);
	if (w) {
		float *aux_x = &w->getChild("line_atlas_left")->getBarParams()->_ratio;
		*aux_x = 0.0f;
		EngineLerp.lerpElement(aux_x, 1.0f, 4.0f, 5.0f);

		float *aux_x_r = &w->getChild("line_atlas_right")->getBarParams()->_ratio;
		*aux_x_r = 0.0f;
		EngineLerp.lerpElement(aux_x_r, 1.0f, 4.0f, 5.0f);

	}
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::ATLAS_LAST_SPLASH_SUB)->makeChildsFadeIn(2.0, 9, false);

	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN)->makeChildsFadeIn(2,16,false);
	EngineLogic.execScriptDelayed("activateCredits();",18.5);
	//EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_CREDITS_BACKGROUND)->makeChildsFadeIn(2, 10, true);
	//EngineLogic.execScriptDelayed("removeAtlasSplash()",12.25);
	//EngineLogic.execScriptDelayed("removeTempCredits()", 25.25);
	//EngineLogic.execScriptDelayed("changeGamestate(\"main_menu\")",25.5);
}

void activateCredits() {
	EngineLogic.execScript("unloadScene();");
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::ATLAS_LAST_SPLASH);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::ATLAS_LAST_SPLASH_LINE);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::ATLAS_LAST_SPLASH_SUB);
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN)->makeChildsFadeOut(0.25, 0.0, false);
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::CREDITS_BACKGROUND);
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::CREDITS);
	EngineLogic.execScriptDelayed("removeAtlasSplash()", 57.0f);
	EngineLogic.execScriptDelayed("changeGamestate(\"main_menu\")", 57.0f);
	EngineLogic.execScript("restartCinematics()");
}

void removeAtlasSplash() {
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::CREDITS);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::CREDITS_BACKGROUND);
}

void tempCredits() {
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_CREDITS_BACKGROUND)->makeChildsFadeIn(2,0,true);
}

void removeTempCredits() {
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_CREDITS_BACKGROUND);
}
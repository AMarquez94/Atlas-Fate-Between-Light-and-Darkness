#include "mcv_platform.h"
#include "module_scene_manager.h"
#include "handle/handle.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "modules/game/module_game_manager.h"
#include "components/comp_group.h"
#include "components/lighting/comp_light_point.h"
#include "components/comp_render.h"
#include "components/comp_transform.h"
#include "render/texture/material.h"
#include "render/render_objects.h"
#include "resources/json_resource.h"
#include "components/comp_tags.h"

// for convenience
using json = nlohmann::json;

CModuleSceneManager::CModuleSceneManager(const std::string& name)
    : IModule(name)
{}

/* Pre-load all the scenes from boot.json */
void CModuleSceneManager::loadJsonScenes(const std::string filepath) {

    sceneCount = 0;

    json jboot = Resources.get(filepath)->as<CJsonResource>()->getJson();
    _default_scene = jboot.value("default_scene","scene_intro");

    for (auto it = std::next(jboot.begin(),1); it != jboot.end(); ++it) {
   
        sceneCount++;
        std::string scene_name = it.key();
        std::vector< std::string> persistent_subscenes = jboot[scene_name]["persistent_scenes"];
        std::vector< std::string > groups_subscenes = jboot[scene_name]["scene_group"];
        
        // Create the scene and store it
        Scene * scene = createScene(scene_name);
        scene->persistent_subscenes = persistent_subscenes;
        scene->groups_subscenes = groups_subscenes;
        auto& data = jboot[scene_name]["static_data"];
        scene->navmesh = data.value("navmesh", "");
        scene->initial_script_name = data.value("initial_script", "");

        scene->env_fog = data.count("env_fog") ? loadVEC3(data["env_fog"]) : cb_globals.global_fog_env_color;
        scene->ground_fog = data.count("ground_fog") ? loadVEC3(data["ground_fog"]) : cb_globals.global_fog_color;
        scene->shadow_color = data.count("shadow_color") ? loadVEC3(data["shadow_color"]) : cb_globals.global_shadow_color;

        scene->env_fog_density = data.value("env_fog_density", cb_globals.global_fog_density);
        scene->ground_fog_density = data.value("ground_fog_density", cb_globals.global_fog_ground_density);

        scene->scene_exposure = data.value("exposure", cb_globals.global_exposure_adjustment);
        scene->scene_ambient = data.value("ambient", cb_globals.global_ambient_adjustment);
        scene->scene_gamma = data.value("gamma", cb_globals.global_gamma_correction_enabled);
        scene->scene_tone_mapping = data.value("tone_mapping", cb_globals.global_tone_mapping_mode);
        scene->scene_shadow_intensity = data.value("shadow_intensity", cb_globals.global_shadow_intensity);

        _scenes.insert(std::pair<std::string, Scene*>(scene_name, scene));
    }
}

bool CModuleSceneManager::start() {

    // Load a persistent scene and the listed ones
    // Store at persistent scene, inviolable data.
    next_scene = "";
    _persistentScene = createScene("Persistent_Scene");
    _persistentScene->isLoaded = true;

    loadJsonScenes("data/boot.json");

    return true;
}

bool CModuleSceneManager::stop() {
    next_scene = "";
    unLoadActiveScene();

    return true;
}

void CModuleSceneManager::update(float delta) {

    if (next_scene != "") {
        if (!EngineFiles.areResourcesToLoad()) {
            loadPartialScene(next_scene);
            next_scene = "";
        }
    }
    // TO-DO, Maybe not needed
}

Scene* CModuleSceneManager::createScene(const std::string& name) {

    Scene* scene = new Scene();
    scene->name = name;
    scene->navmesh = "UNDEFINED";
    scene->initial_script_name = "UNDEFINED";
    scene->isLoaded = false;

    return scene;
}

/* Method used to load a listed scene (must be in the database) */
bool CModuleSceneManager::loadScene(const std::string & name) {

    auto it = _scenes.find(name);
    if (it != _scenes.end())
    {
        // Send a message to notify the scene loading.
        // Useful if we want to show a load splash menu

        if (_activeScene != nullptr && _activeScene->name != name) {
            /* If the new scene is different from the actual one => delete checkpoint */
            CModuleGameManager gameManager = CEngine::get().getGameManager();
            gameManager.deleteCheckpoint();
        }

        unLoadActiveScene(false, name);

        // Load the subscene
        Scene * current_scene = it->second;
        if (current_scene->navmesh.compare("") != 0) {
            Engine.getNavmeshes().buildNavmesh(current_scene->navmesh);
        }

        for (auto& scene_name : current_scene->persistent_subscenes) {
            dbg("Autoloading persistent scene %s\n", scene_name.c_str());
            TEntityParseContext ctx;
            ctx.persistent = true;
            parseScene(scene_name, ctx);
        }

        for (auto& scene_name : current_scene->groups_subscenes) {
            dbg("Autoloading scene %s\n", scene_name.c_str());
            TEntityParseContext ctx;
            parseScene(scene_name, ctx);
        }

        // Renew the active scene
        current_scene->isLoaded = true;
        setActiveScene(current_scene);

        // Move this to LUA.
        CHandle h_camera = getEntityByName("TPCamera");
        if (h_camera.isValid())
            Engine.getCameras().setDefaultCamera(h_camera);

        h_camera = getEntityByName("main_camera");
        if (h_camera.isValid())
            Engine.getCameras().setOutputCamera(h_camera);

        TMsgSceneCreated msg;
        EngineEntities.broadcastMsg(msg);

		CModuleGameManager gameManager = CEngine::get().getGameManager();
		/* TODO: Comprobar que se sigue en la misma escena */
		gameManager.loadCheckpoint();
        Engine.getLogic().execEvent(EngineLogic.SCENE_START, current_scene->name);

        // Set the global data.
        cb_globals.global_fog_color = current_scene->ground_fog;
        cb_globals.global_fog_env_color = current_scene->env_fog;
        cb_globals.global_fog_density = current_scene->env_fog_density;
        cb_globals.global_fog_ground_density = current_scene->ground_fog_density;
        cb_globals.global_exposure_adjustment = current_scene->scene_exposure;
        cb_globals.global_ambient_adjustment = current_scene->scene_ambient;
        cb_globals.global_gamma_correction_enabled = current_scene->scene_gamma;
        cb_globals.global_tone_mapping_mode = current_scene->scene_tone_mapping;
        cb_globals.global_shadow_intensity = current_scene->scene_shadow_intensity;
        cb_globals.global_shadow_color = current_scene->shadow_color;
        cb_globals.updateGPU();

        return true;
    }

    return false;
}

bool CModuleSceneManager::loadPartialScene(const std::string & name)
{
    if (_activeScene != nullptr && _activeScene->persistent_subscenes.size() > 0 && _activeScene->name != name) {
        auto it = _scenes.find(name);
        if (it != _scenes.end())
        {
            CModuleGameManager gameManager = CEngine::get().getGameManager();
            gameManager.deleteCheckpoint();

            unLoadActiveScene(true, name);

            // Load the subscene
            Scene * current_scene = it->second;
            if (current_scene->navmesh.compare("") != 0) {
                Engine.getNavmeshes().buildNavmesh(current_scene->navmesh);
            }

            for (auto& scene_name : current_scene->groups_subscenes) {
                dbg("Autoloading scene %s\n", scene_name.c_str());
                TEntityParseContext ctx;
                parseScene(scene_name, ctx);
            }

            // Renew the active scene
            current_scene->isLoaded = true;
            setActiveScene(current_scene);

            TMsgSceneCreated msg;

            /* Only new entities */
            EngineEntities.broadcastMsg(msg);

            Engine.getLogic().execEvent(EngineLogic.SCENE_PARTIAL_START, current_scene->name);

            // Set the global data.
            cb_globals.global_fog_color = current_scene->ground_fog;
            cb_globals.global_fog_env_color = current_scene->env_fog;
            cb_globals.global_fog_density = current_scene->env_fog_density;
            cb_globals.global_fog_ground_density = current_scene->ground_fog_density;
            cb_globals.global_exposure_adjustment = current_scene->scene_exposure;
            cb_globals.global_ambient_adjustment = current_scene->scene_ambient;
            cb_globals.global_gamma_correction_enabled = current_scene->scene_gamma;
            cb_globals.global_tone_mapping_mode = current_scene->scene_tone_mapping;
            cb_globals.global_shadow_intensity = current_scene->scene_shadow_intensity;
            cb_globals.global_shadow_color = current_scene->shadow_color;
            cb_globals.updateGPU();

            return true;
        }

        return false;
    }
    else {
        return loadScene(name);
    }
}

bool CModuleSceneManager::unLoadActiveScene(bool partial, const std::string& new_scene) {

    // This will allow us to mantain the gamestate.

    // Get the current active scene
    // Free memory related to non persistent data.
    // Warning: persistent data will need to avoid deletion
    if (_activeScene != nullptr) {

        EngineLogic.clearDelayedScripts();

        if (partial) {

            EngineLogic.execEvent(EngineLogic.SCENE_PARTIAL_END, _activeScene->name);

            /* Clear non persistent entities */
            std::vector<uint32_t> tag;
            tag.push_back(getID("persistent"));
            VHandles non_persistent_entities = CTagsManager::get().getAllEntitiesWithoutTags(tag);
            for (int i = 0; i < non_persistent_entities.size(); i++) {
                non_persistent_entities[i].destroy();
            }
            EngineInstancing.clearInstances();
        }
        else {

            EngineLogic.execEvent(EngineLogic.SCENE_END, _activeScene->name);

            EngineEntities.destroyAllEntities();
            EngineCameras.deleteAllCameras();
            EngineInstancing.clearInstances();
            EngineParticles.killAll();
        }

        EngineIA.clearSharedBoards();
        EngineNavmeshes.destroyNavmesh();

        removeSceneResources(_activeScene->name);
        //if (_activeScene->name != new_scene) {
        //    destroySceneResources(_activeScene->name);
        //}

        _activeScene->isLoaded = false;
        _activeScene = nullptr;

        return true;
    }

    return false;
}

void CModuleSceneManager::changeGameState(const std::string & gamestate)
{
    next_scene = "";
    unLoadActiveScene();
    CEngine::get().getModules().changeGameState(gamestate);
}

/* Some getters and setters */

Scene* CModuleSceneManager::getActiveScene() {

    return _activeScene;
}

Scene* CModuleSceneManager::getSceneByName(const std::string& name) {

    return _scenes[name];
}

void CModuleSceneManager::setActiveScene(Scene* scene) {

    //unLoadActiveScene();
    _activeScene = scene;
}

std::string CModuleSceneManager::getDefaultSceneName() {
    return _default_scene;
}

void CModuleSceneManager::preloadScene(const std::string& sceneName) {
    EngineFiles.addVectorResourceToLoad(EngineFiles.getFileResourceVector(sceneName));
    next_scene = sceneName;

    //const std::vector<std::string> resources = EngineFiles.getFileResourceVector(sceneName);
    //for (int i = 0; i < resources.size(); i++) {
    //    EngineFiles.addResourceToLoad(resources[i]);
    //}
}

void CModuleSceneManager::preloadOnlyScene(const std::string& sceneName) {
    EngineFiles.addVectorResourceToLoad(EngineFiles.getFileResourceVector(sceneName));
}

void CModuleSceneManager::removeSceneResources(const std::string& sceneName) {

    const std::vector<std::string> preresources = EngineFiles.getFileResourceVector(sceneName);
    for (int i = 0; i < preresources.size(); i++) {
        EngineFiles.addPendingResourceFile(preresources[i], false);
    }
}

void CModuleSceneManager::destroySceneResources(const std::string& sceneName) {
    const std::vector<std::string> resources_to_destroy = EngineFiles.getFilesToDestroy(sceneName);
    for (int i = 0; i < resources_to_destroy.size(); i++) {
        Resources.destroy(resources_to_destroy[i]);
    }
}
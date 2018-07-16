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

// for convenience
using json = nlohmann::json;

CModuleSceneManager::CModuleSceneManager(const std::string& name)
    : IModule(name)
{}

/* Pre-load all the scenes from boot.json */
void CModuleSceneManager::loadJsonScenes(const std::string filepath) {

    sceneCount = 0;

    json jboot = loadJson(filepath);
    _default_scene = jboot.value("default_scene","scene_intro");

    for (auto it = std::next(jboot.begin(),1); it != jboot.end(); ++it) {
   
        sceneCount++;
        std::string scene_name = it.key();
        std::vector< std::string > groups_subscenes = jboot[scene_name]["scene_group"];
        
        // Create the scene and store it
        Scene * scene = createScene(scene_name);
        scene->groups_subscenes = groups_subscenes;
        auto& data = jboot[scene_name]["static_data"];
        scene->navmesh = data.value("navmesh", "data/navmeshes/milestone2_navmesh.bin");
        scene->initial_script_name = data.value("initial_script", "");

        _scenes.insert(std::pair<std::string, Scene*>(scene_name, scene));
    }
}

bool CModuleSceneManager::start() {

    // Load a persistent scene and the listed ones
    // Store at persistent scene, inviolable data.
    _persistentScene = createScene("Persistent_Scene");
    _persistentScene->isLoaded = true;

    loadJsonScenes("data/boot.json");

    return true;
}

bool CModuleSceneManager::stop() {

    unLoadActiveScene();

    return true;
}

void CModuleSceneManager::update(float delta) {

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

        unLoadActiveScene();

        // Load the subscene
        Scene * current_scene = it->second;
        Engine.getNavmeshes().buildNavmesh(current_scene->navmesh);

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

        auto om = getObjectManager<CEntity>();
        om->forEach([](CEntity* e) {
            TMsgSceneCreated msg;
            CHandle h_e(e);
            h_e.sendMsg(msg);
        });

		CModuleGameManager gameManager = CEngine::get().getGameManager();
		/* TODO: Comprobar que se sigue en la misma escena */
		gameManager.loadCheckpoint();
        Engine.getLogic().execEvent(EngineLogic.SCENE_START, current_scene->initial_script_name);

        // TO REMOVE.
        // Guarrada maxima color neones
        {
            CHandle p_group = getEntityByName("neones");
            CEntity * parent_group = p_group;
            if (p_group.isValid()) {
                TCompGroup * neon_group = parent_group->get<TCompGroup>();
                for (auto p : neon_group->handles) {
                    CEntity * neon = p;
                    TCompTransform * t_trans = neon->get<TCompTransform>();
                    VEC3 pos = t_trans->getPosition();
                    CEntity * to_catch = nullptr;
                    float maxDistance = 9999999;
                    getObjectManager<TCompLightPoint>()->forEach([pos, &to_catch, &maxDistance](TCompLightPoint* c) {
                        CEntity * ent = CHandle(c).getOwner();
                        TCompTransform * c_trans = ent->get<TCompTransform>();
                        float t_distance = VEC3::Distance(pos, c_trans->getPosition());

                        if (t_distance < maxDistance) {
                            to_catch = ent;
                            maxDistance = t_distance;
                        }
                    });

                    if (to_catch != nullptr) {
                        TCompLightPoint * point_light = to_catch->get<TCompLightPoint>();
                        VEC4 neon_color = point_light->getColor();
                        TCompRender * l_render = neon->get<TCompRender>();
                        l_render->self_color = neon_color;
                        l_render->self_intensity = 10.f;
                        /*for (auto p : l_render->meshes) {
                            for (auto t : p.materials) {
                                CMaterial * mat = const_cast<CMaterial*>(t);
                                mat->setSelfColor(VEC4(1,0,0,1));
                                dbg("changed color");
                            }
                        }*/
                    }
                }
            }
        }

        return true;
    }

    return false;
}

bool CModuleSceneManager::unLoadActiveScene() {

    // This will allow us to mantain the gamestate.

    // Get the current active scene
    // Free memory related to non persistent data.
    // Warning: persistent data will need to avoid deletion
    if (_activeScene != nullptr) {

        EngineEntities.destroyAllEntities();
        EngineCameras.deleteAllCameras();
        EngineIA.clearSharedBoards();
        EngineNavmeshes.destroyNavmesh();
        EngineInstancing.clearInstances();
        EngineParticles.killAll();

        Engine.getLogic().execEvent(EngineLogic.SCENE_END, _activeScene->name);

        _activeScene->isLoaded = false;
        _activeScene = nullptr;

	    /* TODO: Delete checkpoint */

        return true;
    }

    return false;
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
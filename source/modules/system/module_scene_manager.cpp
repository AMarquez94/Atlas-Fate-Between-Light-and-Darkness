#include "mcv_platform.h"
#include "module_scene_manager.h"
#include "handle/handle.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "modules/game/module_game_manager.h"

// for convenience
using json = nlohmann::json;

CModuleSceneManager::CModuleSceneManager(const std::string& name)
	: IModule(name)
{}

/* Pre-load all the scenes from boot.json */
void CModuleSceneManager::loadJsonScenes(const std::string filepath) {

	sceneCount = 0;

	json jboot = loadJson(filepath);
	for (auto it = jboot.begin(); it != jboot.end(); ++it) {

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

		unLoadActiveScene();

		// Load the subscene
		Scene * current_scene = it->second;
		Engine.getNavmeshes().buildNavmesh(current_scene->navmesh);
		for (auto& scene_name : current_scene->groups_subscenes) {
			std::string name2 = scene_name;
			getFileNameFromPath(name2);
			dbg("Autoloading scene %s\n", name2.c_str());
			TEntityParseContext ctx;
			if (parseScene(scene_name, ctx)) {
				EngineLogic.execEvent(CModuleLogic::Events::SCENE_START, name2);
			}
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
		EngineLogic.execEvent(EngineLogic.SCENE_START, current_scene->initial_script_name);

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
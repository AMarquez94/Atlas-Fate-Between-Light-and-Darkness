#include "mcv_platform.h"
#include "module_logic.h"
#include "components/comp_tags.h"
#include "components\comp_transform.h"
#include "components\lighting\comp_light_spot.h"
#include "components\ia\comp_bt_patrol.h"
#include "components\ia\comp_bt_mimetic.h"
#include "components\ia\comp_patrol_animator.h"
#include "components\ia\comp_mimetic_animator.h"
#include "components\comp_group.h"
#include "components\comp_name.h"
#include "components\postfx\comp_render_ao.h"
#include "components\physics\comp_rigidbody.h"
#include "components\comp_fsm.h"
#include <experimental/filesystem>
#include "modules/game/module_game_manager.h"
#include <iostream>
#include "physics\physics_collider.h"
#include "entity\entity_parser.h"

#include "components/lighting/comp_light_dir.h"
#include "components/lighting/comp_light_spot.h"
#include "components/lighting/comp_light_point.h"
#include "components/postfx/comp_render_ao.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include <thread>

using namespace physx;

bool CModuleLogic::start() {
	BootLuaSLB();
	execEvent(Events::GAME_START);
	return true;
}

bool CModuleLogic::stop() {
	delete(s);
	delete(m);
	return true;
}

void CModuleLogic::update(float delta) {
	for (unsigned int i = 0; i < delayedScripts.size(); i++) {
		delayedScripts[i].remainingTime -= delta;
		if (delayedScripts[i].remainingTime <= 0) {
			execScript(delayedScripts[i].script);
			delayedScripts.erase(delayedScripts.begin() + i);
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

/* Where we publish all functions that we want and load all the scripts in the scripts folder */
void CModuleLogic::BootLuaSLB() {

	//Publish all the functions
	publishClasses();
	//Load all the scripts
	loadScriptsInFolder("data/scripts");
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
				if (fileName.substr(fileName.find_last_of(".") + 1) == "lua" &&
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

	SLB::Class< VEC3 >("VEC3", m)
		.constructor<float, float, float>()
		.comment("This is our wrapper of the VEC3 class")
		.property("x", &VEC3::x)
		.property("y", &VEC3::y)
		.property("z", &VEC3::z);

	SLB::Class< TCompTempPlayerController >("PlayerController", m)
		.comment("This is our wrapper of the player controller component")
		.property("inhibited", &TCompTempPlayerController::isInhibited);


	/* Global functions */

	//game hacks
	m->set("pauseGame", SLB::FuncCall::create(&pauseGame));
	m->set("loadScene", SLB::FuncCall::create(&loadScene));
	m->set("unloadScene", SLB::FuncCall::create(&unloadScene));
	m->set("loadCheckpoint", SLB::FuncCall::create(&loadCheckpoint));
	m->set("move", SLB::FuncCall::create(&move));
	m->set("spawn", SLB::FuncCall::create(&spawn));


	//player hacks
	m->set("pausePlayerToggle", SLB::FuncCall::create(&pausePlayerToggle));
	m->set("getPlayerController", SLB::FuncCall::create(&getPlayerController));
	m->set("movePlayer", SLB::FuncCall::create(&movePlayer));
	m->set("staminaInfinite", SLB::FuncCall::create(&staminaInfinite));
	m->set("immortal", SLB::FuncCall::create(&immortal));
	m->set("inShadows", SLB::FuncCall::create(&inShadows));
	m->set("speedBoost", SLB::FuncCall::create(&speedBoost));
	m->set("playerInvisible", SLB::FuncCall::create(&playerInvisible));

	//light hacks
	m->set("spotlightsToggle", SLB::FuncCall::create(&spotlightsToggle));
	m->set("lanternToggle", SLB::FuncCall::create(&lanternToggle));
	m->set("shadowsToggle", SLB::FuncCall::create(&shadowsToggle));
	m->set("cg_drawlights", SLB::FuncCall::create(&cg_drawlights));

	//drawing mode hacks
	m->set("wireframeToggle", SLB::FuncCall::create(&wireframeToggle));
	m->set("collidersToggle", SLB::FuncCall::create(&collidersToggle));


	//postfx hacks
	m->set("postFXToggle", SLB::FuncCall::create(&postFXToggle));


	//camera hacks
	m->set("blendInCamera", SLB::FuncCall::create(&blendInCamera));
	m->set("blendOutCamera", SLB::FuncCall::create(&blendOutCamera));
	m->set("blendOutActiveCamera", SLB::FuncCall::create(&blendOutActiveCamera));

	//system hacks
	m->set("pauseEnemies", SLB::FuncCall::create(&pauseEnemies));
	m->set("deleteEnemies", SLB::FuncCall::create(&deleteEnemies));
	m->set("animationsToggle", SLB::FuncCall::create(&animationsToggle));
	m->set("noClipToggle", SLB::FuncCall::create(&noClipToggle));

	//utilities
	m->set("getConsole", SLB::FuncCall::create(&getConsole));
	m->set("getLogic", SLB::FuncCall::create(&getLogic));
	m->set("execDelayedScript", SLB::FuncCall::create(&execDelayedScript));

	//debug hacks
	m->set("fpsToggle", SLB::FuncCall::create(&fpsToggle));
	m->set("debugToggle", SLB::FuncCall::create(&debugToggle));
	m->set("sendOrderToDrone", SLB::FuncCall::create(&sendOrderToDrone));

	//others
	m->set("bind", SLB::FuncCall::create(&bind));
	m->set("renderNavmeshToggle", SLB::FuncCall::create(&renderNavmeshToggle));
	m->set("playSound2D", SLB::FuncCall::create(&playSound2D));
	m->set("exeShootImpactSound", SLB::FuncCall::create(&exeShootImpactSound));
	m->set("sleep", SLB::FuncCall::create(&sleep));
	m->set("probando", SLB::FuncCall::create(&probando));


}

/* Check if it is a fast format command */
void CModuleLogic::execCvar(std::string& script) {

	// Only backslash commands fetched.
	if (script.find("/") != 0)
		return;

	// Little bit of dirty tricks to achieve same results with different string types.
	script.erase(0, 1);
	int index = script.find_first_of(' ');
	script = index != -1 ? script.replace(script.find_first_of(' '), 1, "(") : script;
	index = script.find_first_of(' ');
	script = index != -1 ? script.replace(script.find_first_of(' '), 1, ",") : script;
	script.append(")");
}

CModuleLogic::ConsoleResult CModuleLogic::execScript(const std::string& script) {
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

		break;
	case Events::TRIGGER_ENTER:
		if (delay > 0) {
			return execScriptDelayed("onTriggerEnter_" + params + "()", delay);
		}
		else {
			return execScript("onTriggerEnter_" + params + "()").success;
		}
		break;
	case Events::TRIGGER_EXIT:
		if (delay > 0) {
			return execScriptDelayed("onTriggerExit_" + params + "()", delay);
		}
		else {
			return execScript("onTriggerExit_" + params + "()").success;
		}
		break;
	case Events::SCENE_START:
		if (delay > 0) {
			return execScriptDelayed("onSceneStart_" + params + "()", delay);
		}
		else {
			return execScript("onSceneStart_" + params + "()").success;
		}
		break;
	case Events::SCENE_END:
		if (delay > 0) {
			return execScriptDelayed("onSceneEnd_" + params + "()", delay);
		}
		else {
			return execScript("onSceneEnd_" + params + "()").success;
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

/* Auxiliar functions */
CModuleLogic * getLogic() { return EngineLogic.getPointer(); }

TCompTempPlayerController * getPlayerController()
{
	TCompTempPlayerController * playerController = nullptr;
	CEntity* e = getEntityByName("The Player");
	if (e) {
		playerController = e->get<TCompTempPlayerController>();
	}
	return playerController;
}
CModuleGameConsole * getConsole() { return EngineConsole.getPointer(); }

void execDelayedScript(const std::string& script, float delay)
{
	EngineLogic.execScriptDelayed(script, delay);
}

void pauseGame(bool pause)
{
	TMsgScenePaused msg;
	msg.isPaused = pause;
	EngineEntities.broadcastMsg(msg);
}

void pausePlayerToggle() {
	CEntity* p = getEntityByName("The Player");	
	TCompTempPlayerController* player = p->get<TCompTempPlayerController>();

	TMsgScenePaused stopPlayer;
	stopPlayer.isPaused = !player->paused;
	EngineEntities.broadcastMsg(stopPlayer);
}

void fpsToggle() {
	Engine.getGameManager().config.drawfps = !Engine.getGameManager().config.drawfps;
}

void debugToggle() {
	EngineRender.setDebugMode(!EngineRender.getDebugMode());
}

void blendInCamera(const std::string & cameraName, float blendInTime)
{
	CHandle camera = getEntityByName(cameraName);
	if (camera.isValid()) {
		EngineCameras.blendInCamera(camera, blendInTime, CModuleCameras::EPriority::TEMPORARY);
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

void staminaInfinite() {
	CHandle h = getEntityByName("The Player");
	TMsgInfiniteStamina msg;
	h.sendMsg(msg);
}

void immortal() {
	CHandle h = getEntityByName("The Player");
	TMsgPlayerImmortal msg;
	h.sendMsg(msg);
}

void inShadows() {
	CHandle h = getEntityByName("The Player");
	TMsgPlayerInShadows msg;
	h.sendMsg(msg);
}

void speedBoost(const float speed) {
	CHandle h = getEntityByName("The Player");
	TMsgSpeedBoost msg;
	msg.speedBoost = speed;
	h.sendMsg(msg);
}

void playerInvisible() {
	CHandle h = getEntityByName("The Player");
	TMsgPlayerInvisible msg;
	h.sendMsg(msg);
	std::vector<CHandle> enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
	for (int i = 0; i < enemies.size(); i++) {
		enemies[i].sendMsg(msg);
	}
}

void spotlightsToggle() {
	//For all spotlights, we change their status. This does not desactivate lanterns even though they are spotlights.
	getObjectManager<TCompLightSpot>()->forEach([](TCompLightSpot* c) {
		c->isEnabled = !c->isEnabled;
	});
	//Now we reactivate lanterns in case it is needed.
	std::vector<CHandle> enemies = CTagsManager::get().getAllEntitiesByTag(getID("patrol"));
	for (int i = 0; i < enemies.size(); i++) {
		CEntity* e = enemies[i];
		TCompAIPatrol* patrol = e->get<TCompAIPatrol>();
		bool lights = patrol->getStartLightsOn();
		TCompGroup* group = e->get<TCompGroup>();
		CHandle lantern = group->getHandleByName("FlashLight");
		if (lantern.isValid()) {
			CEntity* e = lantern;
			TCompLightSpot* patrol_lantern = e->get<TCompLightSpot>();
			patrol_lantern->isEnabled = lights;    //else, we activate the lanterns.
		}
	}

}

void lanternToggle() {
	std::vector<CHandle> enemies = CTagsManager::get().getAllEntitiesByTag(getID("patrol"));
	for (int i = 0; i < enemies.size(); i++) {
		CEntity* e = enemies[i];
		TCompGroup* group = e->get<TCompGroup>();
		CHandle lantern = group->getHandleByName("FlashLight");
		if (lantern.isValid()) {
			CEntity* e = lantern;
			TCompLightSpot* patrol_lantern = e->get<TCompLightSpot>();
			patrol_lantern->isEnabled = !patrol_lantern->isEnabled;
		}
	}
}

void shadowsToggle() {
	EngineRender.setGenerateShadows(!EngineRender.getGenerateShadows());
}

void wireframeToggle() {
	EngineRender.setShowWireframe(!EngineRender.getShowWireframe());

}

void collidersToggle(bool onlyDynamics) {
	//Default function. Draw all colliders
	if (!onlyDynamics) {
		EngineRender.setShowAllColliders(!EngineRender.getShowAllColliders());
	}
	else if (onlyDynamics) {
		//Draw only dynamic colliders
		EngineRender.setShowDynamicColliders(!EngineRender.getShowDynamicColliders());
	}

}

void postFXToggle() {

	//Deactivating rest of postFX
	EngineRender.setGeneratePostFX(!EngineRender.getGeneratePostFX());

	////Deactivating AO
	getObjectManager<TCompRenderAO>()->forEach([&](TCompRenderAO* c) {
		c->setState(!c->getState());
	});
}

void pauseEnemies() {
	std::vector<CHandle> enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
	TMsgAIPaused msg;
	for (int i = 0; i < enemies.size(); i++) {
		enemies[i].sendMsg(msg);
	}
}

void animationsToggle() {
	EngineEntities.setAnimationsEnabled(!EngineEntities.getAnimationsEnabled());
}

void noClipToggle() {

	CHandle h = getEntityByName("The Player");
	TMsgSystemNoClipToggle msg;
	h.sendMsg(msg);

}


void deleteEnemies() {
	//To-Do

}

void move(const std::string& entityName, VEC3 pos, VEC3 lookat) {
	CHandle entity_h = getEntityByName(entityName);
	if (entity_h.isValid()) {
		CEntity* e = entity_h;
		TCompTransform* transform = e->get<TCompTransform>();
		transform->lookAt(pos, lookat);
	}
}

void spawn(const std::string & type, VEC3 pos, VEC3 lookAt) {
	if (type.compare("patrol") == 0) {
		/* Create patrol */
		TEntityParseContext ctxPatrol;
		ctxPatrol.root_transform.setPosition(pos);
		parseScene("data/prefabs/patrol.prefab", ctxPatrol);
		CEntity *ePatrol = ctxPatrol.entities_loaded[0];

		/* Set name */
		TCompName *ePatrolName = ePatrol->get<TCompName>();
		ePatrolName->setName(("Spawned Patrol" + std::to_string(getLogic()->spawnedPatrols)).c_str());
		getLogic()->spawnedPatrols++;

	}
	else if (type.compare("mimetic") == 0) {
		/* Create mimetic */
		TEntityParseContext ctxMimetic;
		ctxMimetic.root_transform.setPosition(pos);
		parseScene("data/prefabs/mimetic.prefab", ctxMimetic);
		CEntity *eMimetic = ctxMimetic.entities_loaded[0];

		/* Set name */
		TCompName *eMimeticName = eMimetic->get<TCompName>();
		eMimeticName->setName(("Spawned Mimetic" + std::to_string(getLogic()->spawnedMimetics)).c_str());
		getLogic()->spawnedMimetics++;
	}

}

void movePlayer(VEC3 pos) {
	CHandle h = getEntityByName("The Player");
	TMsgPlayerMove msg;
	msg.pos = pos;
	h.sendMsg(msg);
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
void loadScene(const std::string &level) {

	EngineScene.loadScene(level);
}

void unloadScene() {

	EngineScene.unLoadActiveScene();
}

void sleep(float time) {
	Sleep(time);
}

void probando(const std::string &level) {
	//std::thread newScene(SceneManager.loadScene);
	//newScene.join();
	/*EngineScene.prepareToLoadScene(level);
	EngineScene.unLoadActiveScene();
	EngineScene.setActiveScene(EngineScene.getSceneByName(level));*/
}

void activateScene(const std::string& scene) {
	//EngineScene.setActiveScene()
}

void loadCheckpoint()
{
	CModuleGameManager gameManager = CEngine::get().getGameManager();
	gameManager.loadCheckpoint();
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

void playSound2D(const std::string& soundName) {
	EngineSound.playSound2D(soundName);
}

void exeShootImpactSound() {
	EngineSound.exeShootImpactSound();
}

void sendOrderToDrone(const std::string & droneName, VEC3 position)
{
	CEntity* drone = getEntityByName(droneName);
	TMsgOrderReceived msg;
	msg.position = position;
	msg.hOrderSource = getEntityByName("The Player");
	drone->sendMsg(msg);
}

void renderNavmeshToggle() {
	EngineNavmeshes.renderNamvesh = !EngineNavmeshes.renderNamvesh;
}
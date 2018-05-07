#include "mcv_platform.h"
#include "module_logic.h"
#include "components/comp_tags.h"
#include "components\lighting\comp_light_spot.h"
#include <experimental/filesystem>


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
}

/* Where we publish all functions that we want and load all the scripts in the scripts folder */
void CModuleLogic::BootLuaSLB()
{
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


	/* Global functions */
	m->set("getConsole", SLB::FuncCall::create(&getConsole));
	m->set("getLogic", SLB::FuncCall::create(&getLogic));
	m->set("execDelayedScript", SLB::FuncCall::create(&execDelayedScript));
	m->set("pauseGame", SLB::FuncCall::create(&pauseGame));
	m->set("blendInCamera", SLB::FuncCall::create(&blendInCamera));
	m->set("blendOutCamera", SLB::FuncCall::create(&blendOutCamera));
	m->set("fpsToggle", SLB::FuncCall::create(&fpsToggle));
	m->set("systemToggle", SLB::FuncCall::create(&systemToggle));
	m->set("movePlayer", SLB::FuncCall::create(&movePlayer));
	m->set("staminaInfinite", SLB::FuncCall::create(&staminaInfinite));
	m->set("immortal", SLB::FuncCall::create(&immortal));
	m->set("inShadows", SLB::FuncCall::create(&inShadows));
	m->set("speedBoost", SLB::FuncCall::create(&speedBoost));
	m->set("playerInvisible", SLB::FuncCall::create(&playerInvisible));
	m->set("spotlightsToggle", SLB::FuncCall::create(&spotlightsToggle));







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
CModuleGameConsole * getConsole() { return EngineConsole.getPointer(); }

CModuleLogic * getLogic() { return EngineLogic.getPointer(); }

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

void fpsToggle() {
	EngineRender.showFPS = !EngineRender.showFPS;
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
	CHandle h = getEntityByName("The Player");
	TMsgSpotlightsToggle msg;
	h.sendMsg(msg);
	std::vector<CHandle> spotlights = CTagsManager::get().getAllEntitiesByTag(getID("light"));
	for (int i = 0; i < spotlights.size(); i++) {
		spotlights[i].sendMsg(msg);
	}
}

void systemToggle(const std::string& system) {
	if (system == "enemies") {
		std::vector<CHandle> enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
		TMsgScenePaused msg;
		for (int i = 0; i < enemies.size(); i++) {
			enemies[i].sendMsg(msg);
		}
	}
}

void movePlayer(const float x, const float y, const float z) {
	CHandle h = getEntityByName("The Player");
	TMsgPlayerMove msg;
	msg.pos = VEC3(x, y, z);
	h.sendMsg(msg);
}
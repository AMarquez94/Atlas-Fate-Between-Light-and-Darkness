#include "mcv_platform.h"
#include "engine.h"

#include "modules/game/module_splash.h"
#include "modules/game/module_main_menu.h"
#include "modules/game/module_gameover.h"
#include "modules/game/module_map_intro.h"
#include "modules/game/module_level_select.h"
#include "modules/game/module_game_manager.h"

//--------------------------------------------------------------------------------------
CEngine& CEngine::get() {

	static CEngine engine;
	return engine;
}

CEngine::CEngine()
	: _module_render("render")
	, _module_entities("entities")
	, _module_ia("ia")
	, _module_input("input")
	, _module_physics("physics")
	, _module_cameras("cameras")
	, _module_fsm("fsm")
	, _module_sound("sound")
	, _module_navmesh("navmeshes")
	, _module_logic("logic")
	, _module_game_console("console")
	, _module_gui("gui")
    , _module_scene_manager("scene_manager")
    , _module_instancing("instancing")
{}

bool CEngine::start() {

	static CModuleSplash module_splash("splash");
	static CModuleGameManager module_game_manager("game_manager");
	static CModuleMainMenu module_main_menu("main_menu");
	static CModuleGameOver module_game_over("game_over");
	static CModuleMapIntro module_map_intro("map_intro");
	static CModuleLevelSelect module_level_select("level_select");
    //static CModuleTestInstancing module_test_instancing("test_instancing");

	_modules.registerSystemModule(&_module_render);
	_modules.registerSystemModule(&_module_entities);
	_modules.registerSystemModule(&_module_ia);
	_modules.registerSystemModule(&_module_input);
	_modules.registerSystemModule(&_module_physics);
	_modules.registerSystemModule(&_module_cameras);
	_modules.registerSystemModule(&_module_fsm);
	_modules.registerSystemModule(&_module_sound);
	_modules.registerSystemModule(&_module_navmesh);
	_modules.registerSystemModule(&_module_game_console);
	_modules.registerSystemModule(&_module_gui);
	_modules.registerSystemModule(&_module_logic);          //Always last to start the ongamestarted event from here
    _modules.registerSystemModule(&_module_scene_manager);
    _modules.registerSystemModule(&_module_instancing);

	_modules.registerGameModule(&module_splash);
	_modules.registerGameModule(&module_game_manager);
	_modules.registerGameModule(&module_main_menu);
	_modules.registerGameModule(&module_game_over);
	_modules.registerGameModule(&module_map_intro);
	_modules.registerGameModule(&module_level_select);
    //_modules.registerGameModule(&module_test_instancing);

	_modules.loadModules("data/modules.json");
	_modules.loadGamestates("data/gamestates.json");

	return _modules.start();
}

bool CEngine::stop() {

	bool ok = true;
	ok &= _modules.stop();
	return ok;
}

void CEngine::update(float delta)
{
	PROFILE_FUNCTION("CEngine::update");
	_modules.update(delta);
}

void CEngine::render()
{
    PROFILE_FUNCTION("CEngine::render");
    _module_render.generateFrame();
}

CModuleGameManager& CEngine::getGameManager() {
    return *(CModuleGameManager*)_modules.getModule("game_manager");
}
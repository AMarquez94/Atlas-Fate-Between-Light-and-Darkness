#include "mcv_platform.h"
#include "module_splash.h"
#include "modules/system/module_gui.h"
#include "input/devices/mouse.h"


bool CModuleSplash::start()
{
    _timer = 27.0f;
    EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::SPLASH_SCREEN);
    //EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN)->makeChildsFadeIn(0.25f,7.75f,false);
    CEngine::get().getRender().setBackgroundColor(1.f, 0.f, 0.f, 1.f);
    EngineScene.preloadOnlyScene("scene_intro");
    return true;
}

void CModuleSplash::update(float delta)
{
    _timer -= delta;
    //dbg("Time atm %f\n", _timer);
    if (_timer <= 0.f)
    {
        EngineScene.changeGameState("main_menu");
    }
}

bool CModuleSplash::stop()
{
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::SPLASH_SCREEN);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN);
	return true;
}
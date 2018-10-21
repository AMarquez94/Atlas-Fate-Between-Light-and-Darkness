#include "mcv_platform.h"
#include "module_splash.h"
#include "modules/system/module_gui.h"
#include "input/devices/mouse.h"


bool CModuleSplash::start()
{
	_max_time = 28.0f;
    _timer = 0.0f;
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::SPLASH_SCREEN);
    CEngine::get().getRender().setBackgroundColor(1.f, 0.f, 0.f, 1.f);
    return true;
}

void CModuleSplash::update(float delta)
{
    _timer += delta;

    if (_timer >= _max_time)
    {
        EngineScene.changeGameState("main_menu");
    }
}

bool CModuleSplash::stop()
{
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::SPLASH_SCREEN);
	return true;
}
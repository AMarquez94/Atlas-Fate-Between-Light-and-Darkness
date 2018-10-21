#include "mcv_platform.h"
#include "module_splash.h"
#include "modules/system/module_gui.h"
#include "input/devices/mouse.h"


bool CModuleSplash::start()
{
	_max_time = 28.0f;
    _timer = 0.0f;
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::SPLASH_SCREEN);
	//EngineGUI.getWidget(CModuleGUI::EGUIWidgets::SPLASH_SCREEN)->makeChildsFadeOut(1.5,4.5,false);
    //EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN)->makeChildsFadeIn(0.25f,7.75f,false);
    CEngine::get().getRender().setBackgroundColor(1.f, 0.f, 0.f, 1.f);
    EngineScene.preloadOnlyScene("scene_intro");
	//EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::SPLASH_BACKGROUND);
	//EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::SPLASH_UPF)->makeChildsFadeIn(1.5, 5.0, false);
	//EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::SPLASH_BABYROBOT)->makeChildsFadeIn(1.5, 12, false);
	//EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::SPLASH_SOFTWARE)->makeChildsFadeIn(1.5, 21.5, false);
	//EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::SPLASH_ENGINE)->makeChildsFadeIn(1.5, 30, false);
	//EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN)->makeChildsFadeIn(2.5, 37.0f , false);
    return true;
}

void CModuleSplash::update(float delta)
{
    _timer += delta;
    //dbg("Time atm %f\n", _timer);
	if (_timer > 9.0f && !upfFadedOut) {
		EngineGUI.getWidget(CModuleGUI::EGUIWidgets::SPLASH_UPF)->makeChildsFadeOut(1.5,0.0,false);
		upfFadedOut = true;
	}

	if (_timer > 20.0f && !babyRobotFadedOut) {
		EngineGUI.getWidget(CModuleGUI::EGUIWidgets::SPLASH_BABYROBOT)->makeChildsFadeOut(1.5, 0.0, false);
		babyRobotFadedOut = true;
	}

	if (_timer > 27.5f && !softwareFadedOut) {
		EngineGUI.getWidget(CModuleGUI::EGUIWidgets::SPLASH_SOFTWARE)->makeChildsFadeOut(1.5, 0.0, false);
		softwareFadedOut = true;
	}

	if (_timer > 35.5f && !engineFadedOut) {
		EngineGUI.getWidget(CModuleGUI::EGUIWidgets::SPLASH_ENGINE)->makeChildsFadeOut(1.5, 0.0, false);
		engineFadedOut = true;
	}

    if (_timer >= _max_time)
    {
        EngineScene.changeGameState("main_menu");
    }
}

bool CModuleSplash::stop()
{
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::SPLASH_BACKGROUND);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::SPLASH_UPF);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::SPLASH_BABYROBOT);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::SPLASH_SOFTWARE);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::SPLASH_ENGINE);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN);
	return true;
}
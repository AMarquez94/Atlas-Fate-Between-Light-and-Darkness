#include "mcv_platform.h"
#include "module_splash.h"
#include "modules/system/module_gui.h"

bool CModuleSplash::start()
{
  _timer = 8.f;
  EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::SPLASH_SCREEN);
  CEngine::get().getRender().setBackgroundColor(1.f, 0.f, 0.f, 1.f);
  return true;
}

void CModuleSplash::update(float delta)
{
  _timer -= delta;
  if (_timer <= 0.f)
  {
    CEngine::get().getModules().changeGameState("main_menu");
  }
}

bool CModuleSplash::stop()
{
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::SPLASH_SCREEN);
	return true;
}
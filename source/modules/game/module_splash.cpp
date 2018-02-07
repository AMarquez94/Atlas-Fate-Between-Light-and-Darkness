#include "mcv_platform.h"
#include "module_splash.h"

bool CModuleSplash::start()
{
  _timer = 3.f;
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
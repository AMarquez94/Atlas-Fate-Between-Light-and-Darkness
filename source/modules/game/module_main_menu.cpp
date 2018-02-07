#include "mcv_platform.h"
#include "module_main_menu.h"

bool CModuleMainMenu::start()
{
  CEngine::get().getRender().setBackgroundColor(0.f, 1.f, 0.f, 1.f);
  return true;
}

void CModuleMainMenu::update(float delta)
{
  if (isPressed(VK_SPACE))
  {
    CEngine::get().getModules().changeGameState("game_over");
  }
}
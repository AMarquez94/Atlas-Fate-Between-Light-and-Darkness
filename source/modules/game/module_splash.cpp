#include "mcv_platform.h"
#include "module_splash.h"
#include "input/devices/mouse.h"


bool CModuleSplash::start()
{
    CEngine::get().getRender().setBackgroundColor(1.f, 1.f, 1.f, 1.f);
    Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
    EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::SPLASH);

    return true;
}

bool CModuleSplash::stop() {
    EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::SPLASH);
    return true;
}

void CModuleSplash::update(float delta)
{
    /* if lerped and resources loaded => change gamestate menu */
}
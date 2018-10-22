#include "mcv_platform.h"
#include "module_loading.h"
#include "modules/system/module_gui.h"


bool CModuleLoading::start()
{
    CEngine::get().getRender().setBackgroundColor(1.f, 0.f, 0.f, 1.f);
    EngineScene.preloadOnlyScene("scene_intro");
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::LOADING_BACKGROUND);
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::LOADING_SPRITE);
    return true;
}

void CModuleLoading::update(float delta)
{
    if (!EngineFiles.areResourcesToLoad())
    {
        EngineScene.changeGameState("splash");
    }
}

bool CModuleLoading::stop()
{
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::LOADING_BACKGROUND);
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::LOADING_SPRITE);
	return true;
}
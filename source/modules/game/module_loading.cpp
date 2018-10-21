#include "mcv_platform.h"
#include "module_loading.h"
#include "modules/system/module_gui.h"


bool CModuleLoading::start()
{
    CEngine::get().getRender().setBackgroundColor(1.f, 0.f, 0.f, 1.f);
    EngineScene.preloadOnlyScene("scene_intro");
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
	return true;
}
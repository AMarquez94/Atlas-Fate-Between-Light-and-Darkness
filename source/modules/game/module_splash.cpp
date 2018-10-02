#include "mcv_platform.h"
#include "module_splash.h"

bool CModuleSplash::start()
{
    CEngine::get().getRender().setBackgroundColor(1.f, 1.f, 1.f, 1.f);

    return true;
}

bool CModuleSplash::stop() {
    return true;
}

void CModuleSplash::update(float delta)
{

}
#include "mcv_platform.h"
#include "module_credits.h"
#include "windows/app.h"
#include "input/devices/mouse.h"
#include "render/render_objects.h"

bool CModuleCredits::start()
{
	CEngine::get().getRender().setBackgroundColor(1.f, 1.f, 1.f, 1.f);

	Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
	mouse->setLockMouse(false);

	//EngineScene.loadScene("default");
	EngineGUI.deactivateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN);
	EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN)->makeChildsFadeOut(0.25,0);
	EngineLogic.execSystemScriptDelayed("takeOutBlackScreen();", 0.5f);
	CEngine::get().getGUI().activateWidget(CModuleGUI::EGUIWidgets::INGAME_MENU_PAUSE);
	CEngine::get().getGUI().activateWidget(CModuleGUI::EGUIWidgets::CREDITS);
	CEngine::get().getGUI().activateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN)->makeChildsFadeIn(2,10);
	EngineLogic.execSystemScriptDelayed("takeOutCredits();", 13.0f);
	EngineLogic.execSystemScriptDelayed("goToMainMenu();",14.0f);
	EngineLogic.execSystemScriptDelayed("takeOutBlackScreen();", 14.5f);
	
	CHandle h_camera = getEntityByName("test_camera_flyover");
	if (h_camera.isValid())
		Engine.getCameras().setDefaultCamera(h_camera);

	h_camera = getEntityByName("main_camera");
	if (h_camera.isValid())
		Engine.getCameras().setOutputCamera(h_camera);

	//cb_light.activate();
	//cb_object.activate();
	//cb_camera.activate();
	//Engine.getSound().setAmbientSound("../sounds/test.ogg");

	return true;
}

bool CModuleCredits::stop() {
	CEngine::get().getGUI().deactivateWidget(CModuleGUI::EGUIWidgets::BLACK_SCREEN);
	return true;
}

void CModuleCredits::render()
{

}

void CModuleCredits::update(float delta)
{

}
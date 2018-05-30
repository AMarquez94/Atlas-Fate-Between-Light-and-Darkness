#include "mcv_platform.h"
#include "module_main_menu.h"
#include "windows/app.h"
#include "input/devices/mouse.h"
#include "render/render_objects.h"

bool CModuleMainMenu::start()
{
	CEngine::get().getRender().setBackgroundColor(1.f, 1.f, 1.f, 1.f);
  
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_width = 200;
	window_height = 150;
	
	// Initialization settings
	Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
	mouse->setLockMouse(false);

	CEngine::get().getGUI().activateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_BACKGROUND);
	CEngine::get().getGUI().activateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_BUTTONS);

	CHandle h_camera = getEntityByName("TPCamera");
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

bool CModuleMainMenu::stop() {

	dbg("MODULE STOP\n");
	CEngine::get().getGUI().deactivateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_BACKGROUND);
	CEngine::get().getGUI().deactivateWidget(CModuleGUI::EGUIWidgets::MAIN_MENU_BUTTONS);
	return true;
}

void CModuleMainMenu::render()
{
	/*
	float background_value = abs(sin(time)) * 0.3f + 0.3f;
	CEngine::get().getRender().setBackgroundColor(background_value, background_value, background_value, 1.f);

	VEC2 menu_position = VEC2(CApp::get().xres * .5f - (window_width * .5f),
		CApp::get().yres * .5f - (window_height * .5f));

	ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height));
	ImGui::Begin("MENU", false, window_flags);
	ImGui::CaptureMouseFromApp(false);
	ImGui::SetWindowPos("MENU", ImVec2(menu_position.x, menu_position.y));
	ImGui::Selectable("New Game", menu_load == 0);
	if (ImGui::IsItemClicked() || (menu_load == 0 && EngineInput["btMenuConfirm"].getsPressed()))
	{
		dbg("changed level state\n");
		CEngine::get().getModules().changeGameState("map_intro");
	}

	ImGui::Selectable("Select level", menu_load == 1);
	if (ImGui::IsItemClicked() || (menu_load == 1 && EngineInput["btMenuConfirm"].getsPressed()))
	{
		dbg("changed select state\n");
		CEngine::get().getModules().changeGameState("level_select");
	}

	ImGui::Selectable("Controls", menu_load == 2);
	if (ImGui::IsItemClicked() || (menu_load == 2 && EngineInput["btMenuConfirm"].getsPressed()))
	{
		dbg("changed controls state\n");
	}

	ImGui::Selectable("Options", menu_load == 3);
	if (ImGui::IsItemClicked() || (menu_load == 3 && EngineInput["btMenuConfirm"].getsPressed()))
	{
		dbg("changed options state\n");
	}

	ImGui::Selectable("Credits", menu_load == 4);
	if (ImGui::IsItemClicked() || (menu_load == 4 && EngineInput["btMenuConfirm"].getsPressed()))
	{
		dbg("changed credits state\n");
	}

	ImGui::Selectable("Exit game", menu_load == 5);
	if (ImGui::IsItemClicked() || (menu_load == 5 && EngineInput["btMenuConfirm"].getsPressed()))
	{
		dbg("changed exit state\n");
		exit(0);
	}

	ImGui::End();*/
}

void CModuleMainMenu::update(float delta)
{
	time += delta * transition_speed;

	if (EngineInput["btUpAux"].getsPressed()) {
		menu_load = (menu_load - 1) % menu_size;
	}

	if (EngineInput["btDownAux"].getsPressed()) {
		menu_load = (menu_load + 1) % menu_size;
	}
}
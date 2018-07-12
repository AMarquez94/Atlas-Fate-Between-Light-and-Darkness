#include "mcv_platform.h"
#include "module_gui.h"
#include "render/render_objects.h"
#include "gui/gui_parser.h"
#include "gui/controllers/gui_menu_buttons_controller.h"
#include "gui/widgets/gui_bar.h"
#include "gui/gui_controller.h"

using namespace GUI;

CModuleGUI::CModuleGUI(const std::string& name)
	: IModule(name)
{}

bool CModuleGUI::start()
{
	const float width = 1080;
	const float height = 640;
	_orthoCamera.setOrthographicGUI(width, height);

	_technique = Resources.get("gui.tech")->as<CRenderTechnique>();
	_quadMesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
	_fontTexture = Resources.get("data/textures/gui/font.dds")->as<CTexture>();

	initializeWidgetStructure();
	
	return true;
}

void CModuleGUI::initializeWidgetStructure() {

	//Initializing all the functions for the buttons of GUI
	auto mm_newGameCB = []() {
		CEngine::get().getModules().changeGameState("map_intro");
	};
	auto mm_continueCB = []() {
		//CEngine::get().getGUI().outOfMainMenu();
	};
	auto mm_optionsCB = []() {
		//activateWidget("main_menu_buttons");
	};
	auto mm_exitCB = []() {
		exit(0);
	};
	 
	CMenuButtonsController* mmc = new CMenuButtonsController();

	
	registerWigdetStruct(EGUIWidgets::MAIN_MENU_BACKGROUND, "data/gui/main_menu_background.json");

	registerWigdetStruct(EGUIWidgets::MAIN_MENU_BUTTONS, "data/gui/main_menu_buttons.json", mmc);
	mmc = (CMenuButtonsController*)getWidgetController(EGUIWidgets::MAIN_MENU_BUTTONS);
	mmc->registerOption("new_game", mm_newGameCB);
	mmc->registerOption("continue", mm_continueCB);
	mmc->registerOption("options", mm_optionsCB);
	mmc->registerOption("exit", mm_exitCB);
	mmc->setCurrentOption(0);

	registerWigdetStruct(EGUIWidgets::INGAME_STAMINA_BAR, "data/gui/ingame.json");

}

void CModuleGUI::registerWigdetStruct(EGUIWidgets wdgt_type, std::string wdgt_path, GUI::CController *wdgt_controller) {

	WidgetStructure wdgt_struct;
	CParser parser;
	wdgt_struct._widgetName = parser.parseFile(wdgt_path);
	wdgt_struct._type = wdgt_type;
	wdgt_struct._widget = getWidget(wdgt_struct._widgetName);
	wdgt_struct._controller = wdgt_controller;
	_widgetStructureMap[wdgt_type] = wdgt_struct;
}

bool CModuleGUI::stop()
{
	return true;
}

void CModuleGUI::update(float delta)
{
	if (EngineInput[VK_DOWN].getsPressed())
	{
		//deactivateWidget(EGUIWidgets::MAIN_MENU_BUTTONS);
	}
	if (EngineInput[VK_UP].getsPressed())
	{
		//activateWidget(EGUIWidgets::MAIN_MENU_BACKGROUND);
	}

	for (auto& wdgt : _activeWidgets)
	{
		wdgt->updateAll(delta);
	}
	for (auto& controller : _controllers)
	{
		controller->update(delta);
	}

}

void CModuleGUI::renderGUI()
{
	for (auto& wdgt : _activeWidgets)
	{
		wdgt->renderAll();
	}
}

void CModuleGUI::registerWidget(CWidget* wdgt)
{
	_registeredWidgets.push_back(wdgt);
}

CWidget* CModuleGUI::getWidget(const std::string& name, bool recursive) const
{
	for (auto& rwdgt : _registeredWidgets)
	{
		if (rwdgt->getName() == name)
		{
			return rwdgt;
		}
	}

	if (recursive)
	{
		for (auto& rwdgt : _registeredWidgets)
		{
			CWidget* wdgt = rwdgt->getChild(name, true);
			if (wdgt)
			{
				return wdgt;
			}
		}
	}

	return nullptr;
}

CWidget* CModuleGUI::getWidget(EGUIWidgets wdgt_type) {
	
	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt_type];
	CWidget* wdgt = wdgt_struct._widget;
	if (wdgt != nullptr) {
		return wdgt;
	}
	return nullptr;
}

GUI::CController* CModuleGUI::getWidgetController(EGUIWidgets wdgt_type) {

	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt_type];
	CController* controller = wdgt_struct._controller;
	return controller;
}

void CModuleGUI::activateWidget(EGUIWidgets wdgt)
{
	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt];
	CWidget* widgt = getWidget(wdgt_struct._widgetName);
	if (widgt)
	{
		_activeWidgets.push_back(widgt);
	}
	if (wdgt_struct._controller != nullptr) {
		registerController(wdgt_struct._controller);
	}
}

void CModuleGUI::deactivateWidget(EGUIWidgets wdgt)
{
	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt];
	CWidget* widgt = getWidget(wdgt_struct._widgetName);
	for (auto it = _activeWidgets.begin(); it != _activeWidgets.end();) {
		if (*it == widgt) {
			_activeWidgets.erase(it);
			break;
		}
		it++;
	}

	if (wdgt_struct._controller != nullptr) {
		unregisterController(wdgt_struct._controller);
	}
}

void CModuleGUI::registerController(GUI::CController* controller)
{
	auto it = std::find(_controllers.begin(), _controllers.end(), controller);
	if (it == _controllers.end())
	{
		_controllers.push_back(controller);
	}
}

void CModuleGUI::unregisterController(GUI::CController* controller)
{
	auto it = std::find(_controllers.begin(), _controllers.end(), controller);
	if (it != _controllers.end())
	{
		_controllers.erase(it);
	}
}

CCamera& CModuleGUI::getCamera()
{
	return _orthoCamera;
}

MVariants& CModuleGUI::getVariables()
{
	return _variables;
}

void CModuleGUI::renderTexture(const MAT44& world, const CTexture* texture, const VEC2& minUV, const VEC2& maxUV, const VEC4& color)
{
	assert(_technique && _quadMesh);

	cb_object.obj_world = world;
	cb_object.obj_color = VEC4(1, 1, 1, 1);
	cb_object.updateGPU();

	cb_gui.minUV = minUV;
	cb_gui.maxUV = maxUV;
	cb_gui.tint_color = color;
	cb_gui.updateGPU();

	_technique->activate();
	if (texture)
		texture->activate(TS_ALBEDO);

	_quadMesh->activateAndRender();
}

void CModuleGUI::renderText(const MAT44& world, const std::string& text, const VEC4& color)
{
	assert(_fontTexture);

	int cellsPerRow = 8;
	float cellSize = 1.f / 8.f;
	char firstCharacter = ' ';
	for (size_t i = 0; i < text.size(); ++i)
	{
		char c = text[i];

		int cell = c - firstCharacter;
		int row = cell / cellsPerRow;
		int col = cell % cellsPerRow;

		VEC2 minUV = VEC2(col * cellSize, row * cellSize);
		VEC2 maxUV = minUV + VEC2(1, 1) * cellSize;
		VEC2 gap = (float)i * VEC2(1, 0);
		MAT44 w = MAT44::CreateTranslation(gap.x, gap.y, 0.f) * world;

		renderTexture(w, _fontTexture, minUV, maxUV, color);
	}
}

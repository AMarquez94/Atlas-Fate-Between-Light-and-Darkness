#pragma once

#include "modules/module.h"
#include "gui/gui_widget.h"
#include "utils/variant.h"
#include "gui/gui_controller.h"

class CModuleGUI : public IModule
{
public:
	enum EGUIWidgets{ MAIN_MENU_BACKGROUND = 0, MAIN_MENU_BUTTONS, MAIN_MENU_OPTIONS, INGAME_STAMINA_BAR, INGAME_MENU_PAUSE, INGAME_MENU_PAUSE_BUTTONS, SOUND_GRAPH, DEAD_MENU_BUTTONS, DEAD_MENU_BACKGROUND, CONTROLS, BACK_BUTTON, DEAD_BUTTON, LOADING_SPRITE, BLACK_SCREEN, INGAME_MENU_PAUSE_LINE, CREDITS, MAIN_MENU_CONTROLS_BACKGROUND, MAIN_MENU_CONTROLS_BACK, MAIN_MENU_CREDITS_BACKGROUND, MAIN_MENU_CREDITS_BACK};

	struct WidgetStructure {
		EGUIWidgets _type;
		std::string _widgetName;
		GUI::CWidget *_widget;
		GUI::CController *_controller;
		bool enabled = false;
	};

    const float width = 1080;
    const float height = 640;

	std::map<EGUIWidgets, WidgetStructure> _widgetStructureMap;
	CModuleGUI(const std::string& name);
	bool start() override;
	bool stop() override;
	void update(float delta) override;
	void renderGUI() override;
	void initializeWidgetStructure();

	// widget management
	void registerWigdetStruct(EGUIWidgets wdgt_type, std::string wdgt_path, GUI::CController *wdgt_controller = nullptr);
	void registerWidget(GUI::CWidget* wdgt);
	GUI::CWidget* getWidget(const std::string& name, bool recursive = false) const;
	GUI::CWidget* getWidget(EGUIWidgets wdgt_type);	
	GUI::CWidget* activateWidget(EGUIWidgets wdgt);
	void deactivateWidget(EGUIWidgets wdgt);
    void enableWidget(const std::string& name, bool status = true);
	bool getWidgetStructureEnabled(EGUIWidgets wdgt);

	void activateController(EGUIWidgets wdgt);
	void deactivateController(EGUIWidgets wdgt);

	// controller management
	void registerController(GUI::CController* controller);
	void unregisterController(GUI::CController* controller);
	GUI::CController* getWidgetController(EGUIWidgets wdgt_type);

	CCamera& getCamera();
	MVariants& getVariables();

	void renderTexture(const MAT44& world, const CTexture* texture, const VEC2& minUV, const VEC2& maxUV, const VEC4& color);
    void renderCustomTexture(const std::string & tech, const MAT44& world, const CTexture* texture, const GUI::ConfigParams & params);
	void renderText(const MAT44& world, const std::string& text, const VEC4& color);

	void setButtonsState(bool state);
	bool getButtonsState();

	void closePauseMenu();
private:
	CCamera _orthoCamera;
	const CRenderTechnique* _technique = nullptr;
	const CRenderMesh* _quadMesh = nullptr;
	const CTexture* _fontTexture = nullptr;

	bool buttons_state = true;

	GUI::VWidgets _registeredWidgets;
	GUI::VWidgets _activeWidgets;
	MVariants _variables;
	GUI::VControllers _controllers;
};

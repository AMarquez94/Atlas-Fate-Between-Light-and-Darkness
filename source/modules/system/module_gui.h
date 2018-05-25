#pragma once

#include "modules/module.h"
#include "gui/gui_widget.h"
#include "utils/variant.h"
#include "gui/gui_controller.h"

class CModuleGUI : public IModule
{
public:
	enum EGUIWidgets{ MAIN_MENU_BACKGROUND = 0, MAIN_MENU_BUTTONS, MAIN_MENU_OPTIONS, INGAME_STAMINA_BAR, INGAME_MENU_PAUSE, INGAME_MENU_PAUSE_BUTTONS};

	struct WidgetStructure {
		EGUIWidgets _type;
		std::string _widgetName;
		GUI::CWidget *_widget;
		GUI::CController *_controller;
	};
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
	void activateWidget(EGUIWidgets wdgt);
	void deactivateWidget(EGUIWidgets wdgt);

	// controller management
	void registerController(GUI::CController* controller);
	void unregisterController(GUI::CController* controller);
	GUI::CController* getWidgetController(EGUIWidgets wdgt_type);

	CCamera& getCamera();
	MVariants& getVariables();

	void renderTexture(const MAT44& world, const CTexture* texture, const VEC2& minUV, const VEC2& maxUV, const VEC4& color);
	void renderText(const MAT44& world, const std::string& text, const VEC4& color);

private:
	CCamera _orthoCamera;
	const CRenderTechnique* _technique = nullptr;
	const CRenderMesh* _quadMesh = nullptr;
	const CTexture* _fontTexture = nullptr;

	GUI::VWidgets _registeredWidgets;
	GUI::VWidgets _activeWidgets;
	MVariants _variables;
	GUI::VControllers _controllers;
};

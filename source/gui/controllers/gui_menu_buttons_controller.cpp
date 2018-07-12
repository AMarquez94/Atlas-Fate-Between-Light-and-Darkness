#include "mcv_platform.h"
#include "gui_menu_buttons_controller.h"
#include "gui/widgets/gui_button.h"

namespace GUI
{
  void CMenuButtonsController::update(float delta)
  {
		  if (EngineInput[VK_DOWN].getsPressed() || EngineInput["btDown"].getsPressed())
		  {
			  setCurrentOption(_currentOption + 1);
		  }
		  if (EngineInput[VK_UP].getsPressed() || EngineInput["btUp"].getsPressed())
		  {
			  setCurrentOption(_currentOption - 1);
		  }
		  if (EngineInput[VK_SPACE].getsPressed() || EngineInput["btMenuConfirm"].getsPressed())
		  {
			  _options[_currentOption].button->setCurrentState(CButton::EState::ST_Pressed);
		  }
		  if (EngineInput[VK_SPACE].getsReleased() || EngineInput["btMenuConfirm"].getsReleased())
		  {
			  _options[_currentOption].button->setCurrentState(CButton::EState::ST_Selected);
			  _options[_currentOption].callback();
		  }
  }

  void CMenuButtonsController::registerOption(const std::string& name, GUICallback cb)
  {
    CWidget* wdgt = Engine.getGUI().getWidget(name, true);
    CButton* bt = dynamic_cast<CButton*>(wdgt);
    if (bt)
    {
      _options.push_back(TOption({bt, cb}));
    }
  }

  void CMenuButtonsController::setCurrentOption(int newOption)
  {
    for (auto& option : _options)
    {
      option.button->setCurrentState(CButton::EState::ST_Idle);
    }

    _currentOption = clamp(newOption, 0, static_cast<int>(_options.size()) - 1);
	if (_currentOption >= 0) {

		_options[_currentOption].button->setCurrentState(CButton::EState::ST_Selected);
	}
  }
}

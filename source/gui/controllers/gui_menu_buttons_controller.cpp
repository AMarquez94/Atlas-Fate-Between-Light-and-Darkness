#include "mcv_platform.h"
#include "gui_menu_buttons_controller.h"
#include "gui/widgets/gui_button.h"

namespace GUI
{
    void CMenuButtonsController::update(float delta)
    {
        VEC2 mouse_pos = getMouseInRange(0, EngineGUI.width, 0, EngineGUI.height);
        bool mouse_active = mouse_pos != last_mouse_pos;
        if (mouse_active) {
            last_mouse_pos = mouse_pos;
            int hoveredButton = getButtonHovered(mouse_pos);
            if (hoveredButton != -1) {
                setCurrentOption(hoveredButton);
            }
        }

        if (EngineInput[VK_DOWN].getsPressed() || EngineInput["btDown"].getsPressed())
        {
            mouse_active = false;
            setCurrentOption(_currentOption + 1);
        }
        if (EngineInput[VK_UP].getsPressed() || EngineInput["btUp"].getsPressed())
        {
            mouse_active = false;
            setCurrentOption(_currentOption - 1);
        }
        if (EngineInput[VK_SPACE].getsPressed() || EngineInput["btMenuConfirm"].getsPressed())
        {
            _options[_currentOption].button->setCurrentState(CButton::EState::ST_Pressed);
        }
        else if (EngineInput["btMouseLClick"].getsPressed()) {
            int hoveredButton = getButtonHovered(mouse_pos);
            if (hoveredButton != -1) {
                _options[hoveredButton].button->setCurrentState(CButton::EState::ST_Pressed);
            }
        }
        if (EngineInput[VK_SPACE].getsReleased() || EngineInput["btMenuConfirm"].getsReleased())
        {
            _options[_currentOption].button->setCurrentState(CButton::EState::ST_Selected);
            _options[_currentOption].callback();
        }
        else if (EngineInput["btMouseLClick"].getsReleased()) {
            int hoveredButton = getButtonHovered(mouse_pos);
            if (hoveredButton != -1 && _options[hoveredButton].button->getCurrentState() == CButton::EState::ST_Pressed) {
                _options[hoveredButton].button->setCurrentState(CButton::EState::ST_Selected);
                _options[hoveredButton].callback();
            }
        }
    }

    void CMenuButtonsController::registerOption(const std::string& name, GUICallback cb)
    {
        CWidget* wdgt = Engine.getGUI().getWidget(name, true);
        CButton* bt = dynamic_cast<CButton*>(wdgt);
        if (bt)
        {
            _options.push_back(TOption({ bt, cb }));
        }
    }

    void CMenuButtonsController::setCurrentOption(int newOption)
    {
        if (newOption != _currentOption) {
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
    int CMenuButtonsController::getButtonHovered(VEC2 mouse_pos)
    {
        bool found = false;
        int index = -1;
        int i = 0;
        for (int i = 0; !found && i < _options.size(); i++) {
            VEC2 pos = _options[i].button->getTParams()->_position;
            VEC2 size = _options[i].button->getTParams()->_size;
            _options[i].button;

            VEC3 bScale = VEC3::Zero;
            QUAT bRot = QUAT();
            VEC3 bPos = VEC3::Zero;

            _options[i].button->getAbsolute().Decompose(bScale, bRot, bPos);

            if (isPointInRectangle(mouse_pos, size, VEC2(bPos.x, bPos.y))) {
                found = true;
                index = i;
            }
        }
        return index;
    }
}

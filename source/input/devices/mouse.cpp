#include "mcv_platform.h"
#include "input/devices/mouse.h"
#include "resources/json_resource.h"

namespace Input
{
	CMouse::CMouse(const std::string& name)
		: IDevice(name) 
	{
		_buttons[MOUSE_LEFT] = false;
		_buttons[MOUSE_MIDDLE] = false;
		_buttons[MOUSE_RIGHT] = false;
		_wheel_delta = 0.f;
		_position = VEC2(0.5f, 0.5f);
        setLockMouse(true);
        const json& j = Resources.get("data/config.json")->as<CJsonResource>()->getJson();
        mouse_sensitivity = j.value("mouse_sensitivity", 1.f);
	}

	void CMouse::updateMouseData(float delta, TInterface_Mouse& data)
	{
		// buttons
		data._buttons[MOUSE_LEFT].update(delta, _buttons[MOUSE_LEFT] ? 1.f : 0.f);
		data._buttons[MOUSE_MIDDLE].update(delta, _buttons[MOUSE_MIDDLE] ? 1.f : 0.f);
		data._buttons[MOUSE_RIGHT].update(delta, _buttons[MOUSE_RIGHT] ? 1.f : 0.f);

		// position
		data._position_delta = (_position_delta + data._position_delta) * .5f; 
		data._position = data._position + data._position_delta;
		_position_delta = VEC2::Zero;	//reset previous position

		//dbg("posx %.f posy %.f\n", _position.x, _position.y);
		//if (!_lock_cursor) _previous_position = _position; // Refactor

		// wheel
		data._wheel_delta = _wheel_delta;
		_wheel_delta = 0.f;
	}

	void CMouse::setButton(EMouseButton bt, bool pressed)
	{
		_buttons[bt] = pressed;
	}

	void CMouse::setPosition(float posX, float posY)
	{
		_position = VEC2(posX, posY);
	}

	void CMouse::setWheelDelta(float delta)
	{
		_wheel_delta += delta;
	}

	void CMouse::setPreviousPosition(float posX, float posY)
	{
		_previous_position = VEC2(posX, posY);
	}

	void CMouse::setPositionDelta(float deltaX, float deltaY)
	{
		_position_delta = VEC2(deltaX, deltaY) * mouse_sensitivity;
	}

	void CMouse::setLockMouse(bool locked)
	{
        if (!locked) {
          while (ShowCursor(true) <= 1);
        }
        else {
          while (ShowCursor(false) >= 0);
        }
		
        _lock_cursor = locked;
	}

    void CMouse::setOnlyLockMouse(bool locked) {
        _lock_cursor = locked;
    }

}
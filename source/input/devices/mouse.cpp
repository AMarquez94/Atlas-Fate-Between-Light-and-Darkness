#include "mcv_platform.h"
#include "input/devices/mouse.h"

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
	}

	void CMouse::updateMouseData(float delta, TInterface_Mouse& data)
	{
		// buttons
		data._buttons[MOUSE_LEFT].update(delta, _buttons[MOUSE_LEFT] ? 1.f : 0.f);
		data._buttons[MOUSE_MIDDLE].update(delta, _buttons[MOUSE_MIDDLE] ? 1.f : 0.f);
		data._buttons[MOUSE_RIGHT].update(delta, _buttons[MOUSE_RIGHT] ? 1.f : 0.f);

		// position
		data._position_delta = _position - data._position;
		data._position = _position;

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

}
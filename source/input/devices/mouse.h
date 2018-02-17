#pragma once

#include "input/device.h"

namespace Input
{
	class CMouse : public IDevice
	{
	public:
		bool _lock_cursor;

		CMouse(const std::string& name);
		virtual void updateMouseData(float delta, TInterface_Mouse& data) override;

		void setButton(EMouseButton bt, bool pressed);
		void setPosition(float posX, float posY);
		void setWheelDelta(float delta);

		void setPreviousPosition(float posX, float posY);
		void setLockMouse();

	private:
		bool _buttons[MOUSE_BUTTONS];
		VEC2 _position;
		VEC2 _previous_position;
		float _wheel_delta;
	};
}
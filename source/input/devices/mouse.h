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
        VEC2 getPosition() { return _position; };
		void setPosition(float posX, float posY);
		void setWheelDelta(float delta);

		void setPreviousPosition(float posX, float posY);
		void setPositionDelta(float deltaX, float deltaY);
		void setLockMouse(bool locked);
		bool isMouseLocked() { return _lock_cursor; };

	private:
		bool _buttons[MOUSE_BUTTONS];
		VEC2 _position;
		VEC2 _previous_position;
		VEC2 _position_delta;
		float _wheel_delta;
	};
}
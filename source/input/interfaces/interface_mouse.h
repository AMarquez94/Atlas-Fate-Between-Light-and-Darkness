#pragma once

#include "mcv_platform.h"
#include "input/button.h"

namespace Input
{
	struct TInterface_Mouse
	{
		TButton _buttons[MOUSE_BUTTONS];
		VEC2 _position;
		VEC2 _position_delta;
		float _wheel_delta;

		const TButton& button(EMouseButton bt) const
		{
			return _buttons[bt];
		}
	};
}

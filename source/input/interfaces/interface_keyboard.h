#pragma once

#include "mcv_platform.h"
#include "input/button.h"

namespace Input
{
	struct TInterface_Keyboard
	{
	public:
		TButton _keys[NUM_KEYBOARD_KEYS];

		const TButton& key(int idx) const
		{
			return _keys[idx];
		}
	};
}

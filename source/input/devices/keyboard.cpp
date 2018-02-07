#include "mcv_platform.h"
#include "input/devices/keyboard.h"

namespace Input
{
	void CKeyboard::updateKeyboardData(float delta, TInterface_Keyboard& data)
	{
		for (int i = 0; i < Input::NUM_KEYBOARD_KEYS; ++i)
		{
			bool ok = isPressed(i);
			data._keys[i].update(delta, ok ? 1.f : 0.f);
		}
	}
}
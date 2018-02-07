#pragma once

#include "input/device.h"

namespace Input
{
	class CKeyboard : public IDevice
	{
	public:
		CKeyboard(const std::string& name) : IDevice(name) {}
		virtual void updateKeyboardData(float delta, TInterface_Keyboard& data) override;
	};
}
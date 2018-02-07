#pragma once

#include "mcv_platform.h"
#include "interfaces/interface_keyboard.h"
#include "interfaces/interface_mouse.h"
#include "interfaces/interface_pad.h"
#include "interfaces/interface_feedback.h"

namespace Input
{
	class IDevice
	{
	public:
		IDevice(const std::string& name) : _name(name) {}
		const std::string& getName() const { return _name; }

		virtual void updateKeyboardData(float delta, TInterface_Keyboard& data) { (void)delta;  (void)data; }
		virtual void updateMouseData(float delta, TInterface_Mouse& data) { (void)delta;  (void)data; }
		virtual void updatePadData(float delta, TInterface_Pad& data) { (void)delta;  (void)data; }
		virtual void feedback(const TInterface_Feedback& data) { (void)data; }

	private:
		std::string _name;
	};

	using VDevices = std::vector<IDevice*>;
}

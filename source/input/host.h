#pragma once

#include "device.h"
#include "interfaces/interface_keyboard.h"
#include "interfaces/interface_mouse.h"
#include "interfaces/interface_pad.h"
#include "interfaces/interface_feedback.h"
#include "interfaces/interface_mapping.h"

namespace Input
{
	class CMapping;

	class CHost
	{
	public:
		CHost();

		void update(float delta);
		void assignDevice(IDevice* device);
		void assignMapping(CMapping* mapping);

		const TInterface_Keyboard& keyboard() const { return _keyboard; }
		const TInterface_Mouse& mouse() const { return _mouse; }
		const TInterface_Pad& pad() const { return _pad; }
		const TInterface_Mapping& mapping() const { return _mapping; }
		const TButton& button(const TButtonDef& def) const;
		const VDevices& devices() const { return _devices; }

		void feedback(const TInterface_Feedback& data) const;

	private:
		TInterface_Keyboard _keyboard;
		TInterface_Mouse _mouse;
		TInterface_Pad _pad;
		TInterface_Mapping _mapping;

		VDevices _devices;
	};
}

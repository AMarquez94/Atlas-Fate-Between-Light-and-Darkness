#include "mcv_platform.h"
#include "host.h"
#include "mapping.h"

namespace Input
{
	CHost::CHost()
		: _mapping(*this)
	{}

	void CHost::update(float delta)
	{
		for (auto& device : _devices)
		{
			device->updateKeyboardData(delta, _keyboard);
			device->updateMouseData(delta, _mouse);
			device->updatePadData(delta, _pad);
		}

		_mapping.update(delta);
	}

	void CHost::assignDevice(IDevice* device)
	{
		_devices.push_back(device);
	}

	void CHost::assignMapping(CMapping* mapping)
	{
		_mapping.clear();
		if (mapping)
		{
			_mapping.assignMapping(*mapping);
		}
	}

	void CHost::feedback(const TInterface_Feedback& data) const
	{
		for (auto& device : _devices)
		{
			device->feedback(data);
		}
	}

	const TButton& CHost::button(const TButtonDef& def) const
	{
		static TButton dummy;

		switch(def.type)
		{
			case KEYBOARD:		return _keyboard.key(def.id);
			case MOUSE:				return _mouse.button(EMouseButton(def.id));
			case PAD:					return _pad.button(EPadButton(def.id));
			default:					return dummy;
		}
	}
}

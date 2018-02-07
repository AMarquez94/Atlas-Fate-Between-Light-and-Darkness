#include "mcv_platform.h"
#include "interface_mapping.h"
#include "input/mapping.h"

namespace Input
{
	TInterface_Mapping::TInterface_Mapping(CHost& owner)
		: _host(owner)
	{}

	void TInterface_Mapping::clear()
	{
		_buttons.clear();
	}

	void TInterface_Mapping::update(float delta)
	{
		for (auto& mappedButton : _buttons)
		{
			TMappedButton& mappedBt = mappedButton.second;

			float value = 0.f;
			bool all = true;
			for (auto& bt : mappedBt.buttons)
			{
				all &= bt->isPressed();
				if (std::abs(bt->value) > std::abs(value))
				{
					value = bt->value;
				}
			}

			if (mappedBt.all && !all)
			{
				value = 0.f;
			}

			mappedBt.result.update(delta, value);
		}
	}

	void TInterface_Mapping::assignMapping(const CMapping& mapping)
	{
		clear();

		for (auto& mapDef : mapping.getDefinitions())
		{
			const TMappingInfo& info = mapDef.second;

			TMappedButton bt;
			bt.all = info.all;
			for (auto& defBt : info.buttons)
			{
				const TButton& hostBt = _host.button(defBt);
				bt.buttons.push_back(&hostBt);
			}

			_buttons[mapDef.first] = bt;
		}
	}

	const TButton& TInterface_Mapping::button(const std::string& name) const
	{
		static TButton dummy;

		auto& bt = _buttons.find(name);
		if (bt != _buttons.end())
		{
			return bt->second.result;
		}
		
		return dummy;
	}
}

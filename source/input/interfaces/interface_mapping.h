#pragma once

#include "input/button.h"

namespace Input
{
	class CHost;
	class CMapping;

	struct TInterface_Mapping
	{
		struct TMappedButton
		{
			std::vector<const TButton*> buttons;
			bool all = false;
			TButton result;
		};
		using MButtons = std::map<std::string, TMappedButton>;

		TInterface_Mapping(CHost& owner);
		void update(float delta);
		void clear();
		void assignMapping(const CMapping& mapping);

		const TButton& button(const std::string& name) const;
		const MButtons& buttons() const { return _buttons; }

	private:
		CHost& _host;
		MButtons _buttons;
	};
}

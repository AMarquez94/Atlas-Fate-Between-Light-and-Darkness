#pragma once

#include "input/button.h"

namespace Input
{
	class CHost;

	struct TMappingInfo
	{
		std::vector<TButtonDef> buttons;
		bool all = false;
	};

	class CMapping
	{
	public:
		using MMappingInfo = std::map<std::string, TMappingInfo>;

	public:
		CMapping();
		CMapping(const std::string& filename);
		
		const MMappingInfo& getDefinitions() const { return _mappings; }
		void load(const std::string& filename);
	
	private:
		MMappingInfo _mappings;
	};
}

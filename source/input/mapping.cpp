#include "mcv_platform.h"
#include "mapping.h"
#include "utils/json.hpp"
#include <fstream>

// for convenience
using json = nlohmann::json;

namespace Input
{
	CMapping::CMapping() 
	{}

	CMapping::CMapping(const std::string& filename)
	{
		load(filename);
	}

	void CMapping::load(const std::string& filename)
	{
		std::ifstream file_json(filename);
		json json_data;
		file_json >> json_data;

		_mappings.clear();

		for (auto& jsonMapInfo : json_data)
		{

			std::string name = jsonMapInfo["name"].get<std::string>();

			TMappingInfo info;
			info.all = jsonMapInfo["all"].get<bool>();
			auto& btData = jsonMapInfo["buttons"];
			for (auto& bt : btData)
			{
				const std::string btName = bt.get<std::string>();
				const TButtonDef* def = Engine.getInput().getButtonDefinition(btName);
				info.buttons.push_back(*def);
			}

			_mappings[name] = info;
		}
	}
}

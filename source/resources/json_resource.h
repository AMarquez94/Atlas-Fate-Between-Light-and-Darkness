#pragma once

#include "resources/resource.h"

// ----------------------------------------------
class CJsonResource : public IResource {
	json            json_contents;
public:
	bool create(const std::string& name);
	void debugInMenu() override;
	void onFileChanged(const std::string& filename) override;

	const json& getJson() const {
		return json_contents;
	}
};


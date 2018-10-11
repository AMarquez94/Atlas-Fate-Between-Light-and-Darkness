#pragma once

#include "components/comp_base.h"

class TCompBlurActiveController : public TCompBase {
public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
};
#pragma once

#include "components/comp_base.h"



class TCompCapsuleShadow : public TCompBase {
	DECL_SIBLING_ACCESS();
	
	float maxZ;

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
};

#pragma once

#include "components\comp_base.h"

class TCompDynamicCapsules: public TCompBase {

	DECL_SIBLING_ACCESS();

public:

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx); 
	void Init(); 
	void update(float dt);

};

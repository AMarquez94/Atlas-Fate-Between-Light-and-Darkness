#pragma once

#include "components/comp_base.h"
#include "entity/common_msgs.h"


class TCompGameManagerController;

class TCompGameManagerController : public TCompBase {

	CHandle player;
	bool menuVisible;
	int menuPosition = -1;
	int menuSize = 2;

	DECL_SIBLING_ACCESS();

public:

	void debugInMenu();
	void renderDebug();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	
};
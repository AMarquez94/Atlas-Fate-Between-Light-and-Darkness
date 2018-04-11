#pragma once

#include "components/comp_base.h"


class TCompPlayerInput : public TCompBase
{
	/* Trick to save button state values */
	int inhibition = 0;
	bool crouchButton = false;
	bool attackButtonJustPressed = false;

	DECL_SIBLING_ACCESS();

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

private:
	float _time;
};


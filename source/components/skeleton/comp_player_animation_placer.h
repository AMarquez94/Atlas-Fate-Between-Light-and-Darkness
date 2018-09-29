#pragma once

#include "components/comp_base.h"

class TCompPlayerAnimatorPlacer : public TCompBase {

	DECL_SIBLING_ACCESS();

	VEC3 pointPosition;
	QUAT pointRotation;

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	static void registerMsgs();

	VEC3 getPointPosition();
	QUAT getPointRotation();
};
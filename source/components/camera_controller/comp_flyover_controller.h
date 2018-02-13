#pragma once

#include "../comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompFlyOverController : public TCompBase {

	VEC2    cursor;
	VEC3    speed;
	float   rotation_sensibility = deg2rad(65.f) / 150.0f;
	float   pan_sensibility = 0.7f;
	float   speed_reduction_factor = 0.95f;

	DECL_SIBLING_ACCESS();

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
};


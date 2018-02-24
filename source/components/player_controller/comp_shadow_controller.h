#pragma once

#include "../comp_base.h"
#include "geometry/transform.h"
#include "../ia/ai_controller.h"
#include "entity/common_msgs.h"

class TCompShadowController : public TCompBase {

	DECL_SIBLING_ACCESS();
public:

	bool is_shadow;
	float test_amount;
	float test_levels;

	std::vector<VEC3> static_points;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	void Init();

	bool IsPointInShadows(const VEC3 & point);
  
private:
	void GenerateSurroundingPoints(const VEC3 & point);
};
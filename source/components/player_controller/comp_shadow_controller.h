#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "components/ia/ai_controller.h"
#include "entity/common_msgs.h"

class TCompLight;

class TCompShadowController : public TCompBase {

	void onSceneCreated(const TMsgSceneCreated& msg);

	DECL_SIBLING_ACCESS();
public:

	bool is_shadow;
	float test_amount;
	float test_levels;

	std::vector<TCompLight*> static_lights;
	std::vector<TCompCollider*> dynamic_lights;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	void Init();

	bool IsPointInShadows(const VEC3 & point);

	static void registerMsgs();
private:
	void GenerateSurroundingPoints(const VEC3 & point);
};
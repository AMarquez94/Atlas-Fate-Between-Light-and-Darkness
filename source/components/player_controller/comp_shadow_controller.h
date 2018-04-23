#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "components/ia/ai_controller.h"
#include "entity/common_msgs.h"

class TCompShadowController : public TCompBase {

	void onSceneCreated(const TMsgSceneCreated& msg);
	void onEnteredCapsuleShadow(const TMsgEnteredCapsuleShadow& msg);
	void onExitedCapsuleShadow(const TMsgExitedCapsuleShadow& msg);

	DECL_SIBLING_ACCESS();
public:

	float test_amount;
	float test_levels;
	physx::PxQueryFilterData shadowDetectionFilter;

	bool is_shadow;
	bool capsule_shadow;
	std::vector<CHandle> static_lights;
	std::vector<CHandle> dynamic_lights;

	void Init();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	bool IsPointInShadows(const VEC3 & point);

	static void registerMsgs();
private:
	void GenerateSurroundingPoints(const VEC3 & point);
};
#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompShadowController : public TCompBase {

	void onSceneCreated(const TMsgSceneCreated& msg);
    void onPlayerExposed(const TMsgPlayerIlluminated& msg);
    void onPlayerInShadows(const TMsgPlayerInShadows & msg);

	DECL_SIBLING_ACCESS();
public:

	float test_amount;
	float test_levels;
    physx::PxQueryFilterData lightDetectionFilter;
	physx::PxQueryFilterData shadowDetectionFilter;
	physx::PxQueryFilterData shadowDetectionFilterEnemy;
    physx::PxSphereGeometry shadow_sphere;

	bool is_shadow;
    bool hackShadows = false;
	std::vector<CHandle> static_lights;
	std::vector<CHandle> dynamic_lights;
    std::vector<CHandle> enemies_illuminating_me;

	void Init();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	bool IsPointInShadows(const VEC3 & point, bool player = true);

	static void registerMsgs();
private:
	void GenerateSurroundingPoints(const VEC3 & point);
};
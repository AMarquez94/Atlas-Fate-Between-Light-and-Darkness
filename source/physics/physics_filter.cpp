#include "mcv_platform.h"
#include "physics_collider.h"
#include "components/comp_transform.h"
#include "physics/physics_mesh.h"

physx::PxFilterFlags CustomFilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize
)
{
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{
		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		}
		else {
			pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT | physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
		}
		return physx::PxFilterFlag::eDEFAULT;
	}
	return physx::PxFilterFlag::eSUPPRESS;
}

FilterGroup getFilterByName(const std::string & name)
{
	if (strcmp("player", name.c_str()) == 0) {
		return FilterGroup::Player;
	}
	else if (strcmp("enemy", name.c_str()) == 0) {
		return FilterGroup::Enemy;
	}
	else if (strcmp("characters", name.c_str()) == 0) {
		return FilterGroup::Characters;
	}
	else if (strcmp("wall", name.c_str()) == 0) {
		return FilterGroup::Wall;
	}
	else if (strcmp("floor", name.c_str()) == 0) {
		return FilterGroup::Floor;
	}
	else if (strcmp("ignore", name.c_str()) == 0) {
		return FilterGroup::Ignore;
	}
	else if (strcmp("scenario", name.c_str()) == 0) {
		return FilterGroup::Scenario;
	}
	else if (strcmp("movable", name.c_str()) == 0) {
		return FilterGroup::Movable;
	}
	else if (strcmp("ignoreMovable", name.c_str()) == 0) {
		return FilterGroup::IgnoreMovable;
	}
	return FilterGroup::All;
}
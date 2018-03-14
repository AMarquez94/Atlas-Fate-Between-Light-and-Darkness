#pragma once

#include "../comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"

class TCompRigidbody : public TCompBase {

	physx::PxControllerFilters filters;

	DECL_SIBLING_ACCESS();

public:

	struct TConfig
	{
		float mass;
		float drag;
		float angular_drag;

		bool use_gravity;
		bool is_kinematic;
	};

	TConfig config;
	physx::PxActor* actor;
	physx::PxController* controller;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
	void onCreate(const TMsgEntityCreated& msg);

	class CustomQueryFilterCallback : public physx::PxQueryFilterCallback {

	public:
		physx::PxQueryHitType::Enum PxQueryFilterCallback::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags);
		physx::PxQueryHitType::Enum PxQueryFilterCallback::postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) {
			return physx::PxQueryHitType::eTOUCH;
		}
	};

	CustomQueryFilterCallback customQueryFilter;
};
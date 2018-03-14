#pragma once

#include "components/comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"
#include "resources/collider_resource.h"

class TCompCollider : public TCompBase {
	
	VEC3 lastFramePosition;
	VEC3 normal_gravity = VEC3(0, -9.8f, 0);
	std::map<TCompTransform, CHandle> ownHandles;

	physx::PxVec3 velocity = physx::PxVec3(0, 0, 0);
	physx::PxVec3 totalDownForce = physx::PxVec3(0, 0, 0);

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);
	void onTriggerEnter(const TMsgTriggerEnter& msg);
	void onTriggerExit(const TMsgTriggerExit& msg);

	DECL_SIBLING_ACCESS();

public:

	// Collider parameter description
	CPhysicsCollider * collider_resource;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	void Resize(float new_size);
	void SetUpVector(VEC3 new_up);

	const VEC3 & GetUpVector(void) const;
	bool OnTrigger(const CHandle & entity) const;

	static void registerMsgs();
};
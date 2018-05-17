#pragma once

#include "components/comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"

class CPhysicsCollider;
class TCompTransform;

class TCompCollider : public TCompBase {

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);
	void onTriggerEnter(const TMsgTriggerEnter& msg);
	void onTriggerExit(const TMsgTriggerExit& msg);
	void onMovingObject(const TMsgGrabObject& msg);

	CHandle lastMovedObject;

	DECL_SIBLING_ACCESS();

public:

	bool player_inside;
	std::map<uint32_t, TCompTransform*> handles;

	// Collider parameter description
	CPhysicsCollider * config;

	~TCompCollider();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	/* Auxiliar methods */
	enum Controllers {
		ePlayer = 1,
		ePatrol,
		eMimetic,
		eTotal,
		eReserved = -1
	};

	struct result {
		//True if colision found after filtering
		bool colision = false;
		//Handle returned with the actor colliding
		CHandle handle = CHandle();
		//Sweep buffer with the output
		physx::PxSweepBuffer hit;
	};
	result collisionSweep(VEC3 dir, float distance, physx::PxQueryFlags flags, physx::PxQueryFilterData queryFilterData, Controllers controller = eReserved);
	bool collisionDistance(const VEC3 & org, const VEC3 & dir, float maxDistance);
	void setGlobalPose(VEC3 newPos, VEC4 newRotation, bool autowake = false);

	static void registerMsgs();
};
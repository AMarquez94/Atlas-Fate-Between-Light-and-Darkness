#pragma once

#include "modules/module.h"
#include "components/physics/comp_collider.h"
#include "PxPhysicsAPI.h"
#include "physics/physics_filter.h"

class TCompTransform;

class CModulePhysics : public IModule
{
public:

	static const VEC3 gravity;
	static const float gravityMod;
	static physx::PxQueryFilterData defaultFilter;

	CModulePhysics(const std::string& aname) : IModule(aname) { }
	virtual bool start() override;
	virtual void update(float delta) override;
	virtual void render() override;

	// Physx module getters
	physx::PxScene* getPhysxScene() { return gScene; }
	physx::PxPhysics* getPhysxFactory() { return gPhysics; }
	physx::PxControllerManager* getPhysxController() { return mControllerManager; }
	physx::PxCooking* getCooking() { return gCooking; }

	/* Ray casting & related methods*/
	bool Raycast(const VEC3 & origin, const VEC3 & dir, float distance, physx::PxRaycastHit & hit, physx::PxQueryFlag::Enum flag = physx::PxQueryFlag::eSTATIC, physx::PxQueryFilterData filterdata = defaultFilter);
  bool SphereCast(physx::PxGeometry& geometry, physx::PxTransform& transform, std::vector<physx::PxOverlapHit> & hits, physx::PxQueryFilterData filterdata);

private:

	physx::PxPhysics* gPhysics;
	physx::PxDefaultCpuDispatcher* gDispatcher;
	physx::PxDefaultAllocator gDefaultAllocatorCallback;
	physx::PxDefaultErrorCallback gDefaultErrorCallback;

	physx::PxScene* gScene;
	physx::PxMaterial* gMaterial;
	physx::PxCooking * gCooking;

	physx::PxPvd* gPvd;
	physx::PxFoundation* gFoundation;
	physx::PxControllerManager* mControllerManager;

	class CustomSimulationEventCallback : public physx::PxSimulationEventCallback
	{
		virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {};
		virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override {};
		virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override {};
		virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
		virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);
		virtual void onAdvance(const physx::PxRigidBody*const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)override {};

	};

	CustomSimulationEventCallback customSimulationEventCallback;
};

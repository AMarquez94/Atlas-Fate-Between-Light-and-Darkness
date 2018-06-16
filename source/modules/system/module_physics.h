#pragma once

#include "modules/module.h"
#include "components/physics/comp_collider.h"
#include "PxPhysicsAPI.h"
#include "physics/physics_filter.h"

#define VEC3_TO_PXVEC3(VEC3) physx::PxVec3(VEC3.x, VEC3.y, VEC3.z);
#define PXVEC3_TO_VEC3(PxVec3) VEC3(PxVec3.x, PxVec3.y, PxVec3.z);
#define QUAT_TO_PXQUAT(QUAT) physx::PxQuat(QUAT.x, QUAT.y, QUAT.z, QUAT.w);
#define PXQUAT_TO_QUAT(PxQuat) QUAT(PxQuat.x, PxQuat.y, PxQuat.z, PxQuat.w);

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
    bool Raycast(const VEC3 & origin, const VEC3 & dir, float distance, physx::PxRaycastHit & hit, physx::PxQueryFlags flag = physx::PxQueryFlag::eSTATIC, physx::PxQueryFilterData filterdata = defaultFilter);
    bool Overlap(physx::PxGeometry& geometry, VEC3 pos, std::vector<physx::PxOverlapHit> & hits, physx::PxQueryFilterData filterdata);

    /* Other utilities */
    physx::PxFixedJoint* CreateFixedJoint(physx::PxRigidActor* dynamicActor, const physx::PxTransform& dynamicActorTransform, physx::PxRigidActor* otherActor, const physx::PxTransform& otherActorTransform);
    physx::PxDistanceJoint* CreateDistanceJoint(physx::PxRigidActor* dynamicActor, const physx::PxTransform& dynamicActorTransform, physx::PxRigidActor* otherActor, const physx::PxTransform& otherActorTransform);


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

#pragma once

#include "modules/module.h"
#include "components/comp_collider.h"
#include "PxPhysicsAPI.h"

class CModulePhysics : public IModule
{
public:
  CModulePhysics(const std::string& aname) : IModule(aname) { }
  virtual bool start() override;
  virtual void update(float delta) override;
  virtual void render() override;
  void createActor(TCompCollider& comp_collider);

private:
    physx::PxDefaultAllocator gDefaultAllocatorCallback;
    physx::PxDefaultErrorCallback gDefaultErrorCallback;

    physx::PxPhysics*				gPhysics;

    physx::PxDefaultCpuDispatcher*	gDispatcher;
    physx::PxScene*				gScene;

    physx::PxMaterial*				gMaterial;

    physx::PxPvd*                  gPvd;
    physx::PxFoundation*			gFoundation;
    physx::PxControllerManager*     mControllerManager;
    physx::PxCapsuleController* ctrl;
    

    void createStack(const physx::PxTransform& t, physx::PxU32 size, physx::PxReal halfExtent);

  class CustomSimulationEventCallback : public physx::PxSimulationEventCallback
  {
    virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {};
    virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override {};
    virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override {};
    virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override {};
    virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);
    virtual void onAdvance(const physx::PxRigidBody*const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)override {};

  };

  CustomSimulationEventCallback customSimulationEventCallback;
};

#pragma once

#include "modules/module.h"
#include "PxPhysicsAPI.h"

class CModulePhysics : public IModule
{
public:
  CModulePhysics(const std::string& aname) : IModule(aname) { }
  virtual bool start() override;
  virtual void update(float delta) override;
  virtual void render() override;

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


};

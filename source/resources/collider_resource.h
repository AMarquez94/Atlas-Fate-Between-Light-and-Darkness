#pragma once

#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"
#include "resources/resource.h"

struct CPhysicsCollider : public IResource {

	physx::PxVec3 center;
	physx::PxActor* actor;
	physx::PxMaterial* gMaterial;

public:
	bool is_trigger;
};

struct CPhysicsBox : public CPhysicsCollider{

public:
	physx::PxVec3 size;
};

struct CPhysicsSphere : public CPhysicsCollider{

public:
	float radius;
};

struct CPhysicsCapsule : public CPhysicsCollider{

public:
	float height;
	float radius;
	physx::PxVec3 direction;
};

struct CPhysicsConvex : public CPhysicsCollider{

public:
	float height;
	float radius;
	physx::PxVec3 direction;
};


struct CPhysicsMesh : public CPhysicsCollider{

public:
	float height;
	float radius;
	physx::PxVec3 direction;
};

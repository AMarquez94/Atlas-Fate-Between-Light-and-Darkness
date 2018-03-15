#pragma once

#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"
#include "resources/resource.h"

class TCompTransform;

struct CPhysicsMesh : public IResource {

	uint32_t num_vertexs = 0;
	uint32_t num_indices = 0;
	std::vector< uint8_t > vtxs;
	std::vector< uint8_t > idxs;

public:

	bool create(
		  const void* vertex_data
		, const void* index_data
		, uint32_t vertex_size
		, uint32_t index_size);

	void destroy() override;
	void debugInMenu() override;
};

struct CPhysicsCollider {

	physx::PxActor* actor;
	physx::PxMaterial* material;
	physx::PxGeometryType shape_type;

public:

	enum FilterGroup {

		Wall = 1 << 0,
		Floor = 1 << 1,
		Player = 1 << 2,
		Enemy = 1 << 3,
		Ignore = 1 << 4,
		Fence = 1 << 5,
		Scenario = Wall | Floor,
		Characters = Player | Enemy,
		All = -1
	};

	bool is_trigger;

	physx::PxU32 group;
	physx::PxU32 mask;
	physx::PxVec3 center;
	physx::PxF32 contact_offset;

	virtual void load(const json& j, TEntityParseContext& ctx) {}
	virtual void create(TCompTransform * c_transform) {}
	virtual void debugInMenu() {}

	physx::PxActor* getActor(void) { return actor; };
	void setupFiltering(physx::PxShape* shape, physx::PxU32 filterGroup, physx::PxU32 filterMask);
	void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask);
	void setAsTrigger(physx::PxShape * shape, bool state);
};

struct CPhysicsBox : public CPhysicsCollider{

public:
	physx::PxVec3 size;
	void load(const json& j, TEntityParseContext& ctx) override;
	void create(TCompTransform * c_transform) override;
	void debugInMenu() override;
};

struct CPhysicsPlane: public CPhysicsCollider {

public:
	void load(const json& j, TEntityParseContext& ctx) override;
	void create(TCompTransform * c_transform) override;
	void debugInMenu() override;
};


struct CPhysicsSphere : public CPhysicsCollider{

public:
	float radius;
	void load(const json& j, TEntityParseContext& ctx) override;
	void create(TCompTransform * c_transform) override;
	void debugInMenu() override;
};

struct CPhysicsCapsule : public CPhysicsCollider{

public:
	float height;
	float radius;

	void load(const json& j, TEntityParseContext& ctx) override;
	void create(TCompTransform * c_transform) override;
	void debugInMenu() override;
};

struct CPhysicsConvex : public CPhysicsCollider{

	CPhysicsMesh * mesh;
public:
	float height;
	float radius;

	void load(const json& j, TEntityParseContext& ctx) override;
	void create(TCompTransform * c_transform) override;
	void debugInMenu() override;
};


struct CPhysicsTriangleMesh : public CPhysicsCollider{

	CPhysicsMesh * mesh;
public:
	float height;
	float radius;

	void load(const json& j, TEntityParseContext& ctx) override;
	void create(TCompTransform * c_transform) override;
	void debugInMenu() override;
};

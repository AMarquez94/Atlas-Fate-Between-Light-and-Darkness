#pragma once

#include "PxPhysicsAPI.h"
#include "physics/physics_filter.h"

class CPhysicsMesh;
class TCompTransform;

class CPhysicsCollider {

protected:
    physx::PxMaterial* material;
    physx::PxGeometryType shape_type;

public:

    static physx::PxMaterial* default_material;

    bool is_trigger = false;
    bool is_controller = false;
    physx::PxRigidActor* actor;
    physx::PxShape* shape;

    physx::PxU32 group;
    physx::PxU32 mask;
    physx::PxVec3 center = physx::PxVec3(); // move this...
    physx::PxF32 contact_offset = 0.001f;

    virtual physx::PxShape* createShape() = 0;
    virtual void load(const json& j, TEntityParseContext& ctx) = 0;
    virtual physx::PxController* createController(TCompTransform * c_transform) = 0;
    virtual void debugInMenu() = 0;

    void createStatic(physx::PxShape* actor_shape, TCompTransform * c_transform);
    void createDynamic(physx::PxShape* actor_shape, TCompTransform * c_transform, float mass);

    void setupFiltering(physx::PxShape* shape, physx::PxU32 filterGroup, physx::PxU32 filterMask);
    void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask);
    void setAsTrigger(physx::PxShape * shape, bool state);
};

struct CPhysicsBox : public CPhysicsCollider {

public:
    physx::PxVec3 size;
    float step_offset;

    physx::PxShape*  createShape() override;
    physx::PxController* createController(TCompTransform * c_transform) override;
    void load(const json& j, TEntityParseContext& ctx) override;
    void debugInMenu() override;
};

struct CPhysicsPlane : public CPhysicsCollider {

public:

    physx::PxShape*  createShape() override;
    physx::PxController* createController(TCompTransform * c_transform) { fatal("UNSUPPORTED\n"); return nullptr; };
    void load(const json& j, TEntityParseContext& ctx) override;
    void debugInMenu() override;
};

struct CPhysicsSphere : public CPhysicsCollider {

public:
    float radius;

    physx::PxShape*  createShape() override;
    physx::PxController* createController(TCompTransform * c_transform) { fatal("UNSUPPORTED\n"); return nullptr; };
    void load(const json& j, TEntityParseContext& ctx) override;
    void debugInMenu() override;
};

struct CPhysicsCapsule : public CPhysicsCollider {

public:
    float height;
    float radius;
    float step_offset;

    physx::PxShape*  createShape() override;
    physx::PxController* createController(TCompTransform * c_transform) override;
    void load(const json& j, TEntityParseContext& ctx) override;
    void debugInMenu() override;
};

struct CPhysicsConvex : public CPhysicsCollider {

    CPhysicsMesh * mesh;
    std::string filename;
    physx::PxConvexFlags flags;

    physx::PxConvexMeshCookingType::Enum convextype = physx::PxConvexMeshCookingType::eQUICKHULL;

public:
    float height;
    float radius;

    physx::PxShape*  createShape() override;
    physx::PxController* createController(TCompTransform * c_transform) { fatal("UNSUPPORTED\n"); return nullptr; };
    void load(const json& j, TEntityParseContext& ctx) override;
    void debugInMenu() override;
};

struct CPhysicsTriangleMesh : public CPhysicsCollider {

    CPhysicsMesh * mesh;
    std::string filename;
    physx::PxMeshFlags flags;

public:
    float height;
    float radius;

    physx::PxShape*  createShape() override;
    physx::PxController* createController(TCompTransform * c_transform) { fatal("UNSUPPORTED\n"); return nullptr; };
    void load(const json& j, TEntityParseContext& ctx) override;
    void debugInMenu() override;
};

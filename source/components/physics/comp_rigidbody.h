#pragma once

#include "components/comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"

class TCompRigidbody : public TCompBase {

    physx::PxVec3 velocity = physx::PxVec3(0, 0, 0);
    physx::PxVec3 totalDownForce = physx::PxVec3(0, 0, 0);

    void createRigidbody();

    void onCreate(const TMsgEntityCreated& msg);
    void onGroupCreated(const TMsgEntitiesGroupCreated& msg);

    DECL_SIBLING_ACCESS();

public:

    physx::PxRigidBody* rigidbody = nullptr;
    physx::PxController* controller = nullptr;
    physx::PxControllerFilters filters;
    physx::PxJoint* joint = nullptr; /* Only one joint at a time at the moment */

    float mass;
    float drag;
    float angular_drag;

    bool is_gravity;
    bool is_kinematic;
    bool is_controller;
    bool is_grounded;
    bool is_enabled;

    VEC3 offset;
    VEC3 lastFramePosition;
    VEC3 normal_gravity = VEC3(0, -9.8f, 0);

    ~TCompRigidbody();
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    void Resize(float new_size);
    void SetUpVector(VEC3 new_up);
    VEC3 GetUpVector();

    void Jump(VEC3 forceUp);
    void setNormalGravity(VEC3 newGravity);

    void setGlobalPose(VEC3 pos, QUAT rot);

    static void registerMsgs();

    void setLinearVelocity(VEC3 vel, bool autowake = false);
    void addForce(VEC3 force);
    void setKinematic(bool isKinematic);

    void createJoint(CHandle entityToJoin);
    void releaseJoint();

    void createController();
    void createDynamicRigidbody();
    void destroyController();
    void destroyDynamicRigidbody();

    //class CustomQueryFilterCallback : public physx::PxQueryFilterCallback {

    //public:
    //	physx::PxQueryHitType::Enum PxQueryFilterCallback::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags);
    //	physx::PxQueryHitType::Enum PxQueryFilterCallback::postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) {
    //		return physx::PxQueryHitType::eTOUCH;
    //	}
    //};

    //CustomQueryFilterCallback customQueryFilter;
};
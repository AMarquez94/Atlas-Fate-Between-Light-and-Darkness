#include "mcv_platform.h"
#include "comp_rigidbody.h"
#include "comp_collider.h"
#include "components/comp_transform.h"
#include "physics/physics_collider.h"

DECL_OBJ_MANAGER("rigidbody", TCompRigidbody);
bool temp_ground = true;

TCompRigidbody::~TCompRigidbody() {

    // Delete the controller.
    if (is_controller) {
        controller->release();
    }

    // Rigidbody destruction here.
}

void TCompRigidbody::debugInMenu() {

}

void TCompRigidbody::load(const json& j, TEntityParseContext& ctx) {

    mass = j.value("mass", 10.f);
    drag = j.value("mass", 0.f);

    is_gravity = j.value("is_gravity", false);
    is_kinematic = j.value("is_kinematic", false);
    is_controller = j.value("is_controller", false);
}

void TCompRigidbody::update(float dt) {

    if (CHandle(this).getOwner().isValid()) {

        TCompCollider * c_collider = get<TCompCollider>();
        velocity = physx::PxVec3(0, 0, 0);

        TCompTransform *transform = get<TCompTransform>();
        VEC3 new_pos = transform->getPosition();
        VEC3 delta_movement = new_pos - lastFramePosition;
        velocity = physx::PxVec3(delta_movement.x, delta_movement.y, delta_movement.z) / dt;

        physx::PxVec3 actualDownForce = physx::PxVec3(0, 0, 0);

        if (is_controller) {

            if (is_gravity) {
              if (is_grounded) totalDownForce = physx::PxVec3(0, 0, 0);
              actualDownForce = physx::PxVec3(normal_gravity.x, normal_gravity.y, normal_gravity.z);
              velocity += (actualDownForce + totalDownForce);
              totalDownForce += 3.f * actualDownForce * dt;
            }

            physx::PxControllerCollisionFlags col = controller->move(velocity * dt, 0.f, dt, filters);
            is_grounded = col.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);

            /* We handle here the difference between the logical transform (our component transform) and the physx transform */
            physx::PxExtendedVec3 new_pos_transform = controller->getFootPosition();
            VEC3 new_trans_pos = VEC3((float)new_pos_transform.x, (float)new_pos_transform.y, (float)new_pos_transform.z);
            transform->setPosition(new_trans_pos);
            lastFramePosition = new_trans_pos;
        }
        else
        {/*
          if (is_kinematic) {
            TCompCollider* c_collider = get<TCompCollider>();
            physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)c_collider->config->actor;
            physx::PxVec3 pxPos = VEC3_TO_PXVEC3(transform->getPosition());
            physx::PxQuat pxQuat = QUAT_TO_PXQUAT(transform->getRotation());
            physx::PxTransform pxTransform(pxPos, pxQuat);
            actor->setKinematicTarget(pxTransform);
          }*/
            //VEC3 vel = (transform->getPosition() - lastFramePosition) / dt;
            //physx::PxRigidBody * rigidbody = (physx::PxRigidBody*) c_collider->config->actor;
            //const physx::PxVec3 pxVel = VEC3_TO_PXVEC3(vel);
            //rigidbody->setLinearVelocity(pxVel, true);

          //setRigidBodyFlag
            
            VEC3 pos = transform->getPosition() + PXVEC3_TO_VEC3(c_collider->config->center);
            QUAT quat = transform->getRotation();
            c_collider->setGlobalPose(pos, quat, false);

            //if (is_gravity) {
              //physx::PxRigidBody * rigidbody = (physx::PxRigidBody*) c_collider->config->actor;
              //actualDownForce = physx::PxVec3(normal_gravity.x, normal_gravity.y, normal_gravity.z);
              //rigidbody->addForce(actualDownForce, physx::PxForceMode::eFORCE);
            //}
            //c_collider->config.
        }
    }
}

/* Collider/Trigger messages */
void TCompRigidbody::registerMsgs() {
    DECL_MSG(TCompRigidbody, TMsgEntityCreated, onCreate);
}

void TCompRigidbody::setLinearVelocity(VEC3 vel, bool autowake)
{
  if (is_controller) {
    /* We dont support this at the moment */
  }
  else {
    TCompCollider * c_collider = get<TCompCollider>();
    physx::PxRigidBody * rigidbody = (physx::PxRigidBody*) c_collider->config->actor;
    const physx::PxVec3 pxVel = VEC3_TO_PXVEC3(vel);
    rigidbody->setLinearVelocity(pxVel, autowake);
  }
}

void TCompRigidbody::setKinematic(bool isKinematic)
{
  if (!is_controller) {
    TCompCollider* c_collider = get<TCompCollider>();
    physx::PxRigidBody* actor = (physx::PxRigidBody*)c_collider->config->actor;
    actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
    is_kinematic = isKinematic;
  }
}

void TCompRigidbody::createJoint(CHandle entityToJoin)
{
  /* TODO: Not tested */
  if (joint == nullptr) {
    assert(entityToJoin.isValid());
    CEntity* eEntityToJoin = entityToJoin;

    TCompCollider * my_collider = get<TCompCollider>();
    TCompCollider * other_collider = eEntityToJoin->get<TCompCollider>();

    assert(my_collider && other_collider);

    TCompTransform * my_transform = get<TCompTransform>();
    TCompTransform * other_transform = eEntityToJoin->get<TCompTransform>();

    physx::PxVec3 pxPos = VEC3_TO_PXVEC3(my_transform->getPosition());
    physx::PxQuat pxRot = QUAT_TO_PXQUAT(my_transform->getRotation());
    physx::PxTransform myPxTrans = physx::PxTransform(pxPos, pxRot);

    pxPos = VEC3_TO_PXVEC3(other_transform->getPosition());
    pxRot = QUAT_TO_PXQUAT(other_transform->getRotation());
    physx::PxTransform otherPxTrans = physx::PxTransform(pxPos, pxRot);

    joint = EnginePhysics.CreateDistanceJoint(my_collider->config->actor, myPxTrans, other_collider->config->actor, otherPxTrans);
    assert(joint);
  }
}

void TCompRigidbody::releaseJoint()
{
  /* TODO: Not tested */
  if (joint != nullptr) {
    joint->release();
    joint = nullptr;
  }
}

void TCompRigidbody::createController()
{
  TCompCollider * c_collider = get<TCompCollider>();
  TCompTransform * c_transform = get<TCompTransform>();
  lastFramePosition = c_transform->getPosition();

  controller = c_collider->config->createController(c_transform);
  c_collider->config->actor->userData = CHandle(c_collider).asVoidPtr();
  c_collider->config->is_controller = true;
  is_controller = true;
}

void TCompRigidbody::createDynamicRigidbody()
{
  TCompCollider * c_collider = get<TCompCollider>();
  TCompTransform * c_transform = get<TCompTransform>();

  physx::PxShape * shape = c_collider->config->createShape();
  c_collider->config->createDynamic(shape, c_transform, mass);
  c_collider->config->actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !is_gravity);
  c_collider->config->actor->userData = CHandle(c_collider).asVoidPtr();
}

void TCompRigidbody::destroyController() {
  TCompCollider * c_collider = get<TCompCollider>();

  is_controller = false;
  c_collider->config->is_controller = false;
  controller->release();
}

void TCompRigidbody::destroyDynamicRigidbody() {
  TCompCollider * c_collider = get<TCompCollider>();

  c_collider->config->actor->release();
  c_collider->config->actor = nullptr;
}

void TCompRigidbody::onCreate(const TMsgEntityCreated& msg) {

    TCompCollider * c_collider = get<TCompCollider>();
    TCompTransform * compTransform = get<TCompTransform>();

    // Let the rigidbody handle the creation if it exists..
    if (c_collider != nullptr)
    {

        if (is_controller)
        {
          createController();
        }
        else
        {
          createDynamicRigidbody();
        }

        physx::PxFilterData * characterFilterData = new physx::PxFilterData();
        characterFilterData->word0 = c_collider->config->group;
        characterFilterData->word1 = c_collider->config->mask;

        filters = physx::PxControllerFilters();
        //filters.mFilterCallback = &customQueryFilter;
        filters.mFilterData = characterFilterData;
    }

    lastFramePosition = compTransform->getPosition();
}

void TCompRigidbody::Resize(float new_size)
{
    // Cannot resize if no collider is present.
    TCompCollider * c_collider = get<TCompCollider>();

    if (!c_collider || !c_collider->config->actor)
        return;

    if (controller != NULL)
        controller->resize((physx::PxReal)new_size);
}

void TCompRigidbody::SetUpVector(VEC3 new_up)
{
    controller->setUpDirection(physx::PxVec3(new_up.x, new_up.y, new_up.z));
}

VEC3 TCompRigidbody::GetUpVector()
{
    physx::PxVec3 upDirection = controller->getUpDirection();
    return VEC3(upDirection.x, upDirection.y, upDirection.z);
}

void TCompRigidbody::Jump(VEC3 forceUp)
{
    if (is_grounded) {
        totalDownForce = physx::PxVec3(forceUp.x, forceUp.y, forceUp.z);
        is_grounded = false;
    }
}

void TCompRigidbody::setNormalGravity(VEC3 newGravity) {

    normal_gravity = newGravity;
    totalDownForce = physx::PxVec3(0, 0, 0);
}

void TCompRigidbody::setGlobalPose(VEC3 pos, QUAT rot)
{
  TCompCollider * c_collider = get<TCompCollider>();
  if (is_controller) {
    physx::PxExtendedVec3 pxPos = physx::PxExtendedVec3(pos.x, pos.y, pos.z);
    controller->setFootPosition(pxPos);
    lastFramePosition = pos;
  }
  else {
    c_collider->setGlobalPose(pos, rot, false);
  }
}



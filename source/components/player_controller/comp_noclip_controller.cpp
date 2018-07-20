#include "mcv_platform.h"
#include "comp_noclip_controller.h"
#include "components/comp_transform.h"
#include "components/comp_hierarchy.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"
#include "components/ia/comp_bt_patrol.h"
#include "components/ia/comp_bt_mimetic.h"

DECL_OBJ_MANAGER("noclip_controller", TCompNoClipController);

void TCompNoClipController::debugInMenu() {
}

void TCompNoClipController::load(const json& j, TEntityParseContext& ctx) {
    speed = j.value("speed", 5.0f);
    rotationSpeed = j.value("rotationSpeed", 10.f);
}

void TCompNoClipController::registerMsgs()
{
    DECL_MSG(TCompNoClipController, TMsgScenePaused, onMsgScenePaused);
    DECL_MSG(TCompNoClipController, TMsgSystemNoClipToggle, onMsgNoClipToggle);
}

void TCompNoClipController::onMsgNoClipToggle(const TMsgSystemNoClipToggle & msg)
{
    isInNoClipMode = !isInNoClipMode;
    
    CEntity * me = CHandle(this).getOwner();
    TCompRigidbody* rigidbody = me->get<TCompRigidbody>();
    TCompCollider* collider = me->get<TCompCollider>();

    if (isInNoClipMode) {
        /* Prepare the entity to be in no clip mode */
        prevGravity = rigidbody->normal_gravity;
        rigidbody->setNormalGravity(VEC3::Zero);
        //collider->enableCollisionsAndQueries(false);
        //EnginePhysics.getPhysxScene()->removeActor(*(collider->config->actor));
        collider->config->actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
    }
    else {
        /* Prepare the entity to be in normal mode */
        //EnginePhysics.getPhysxScene()->addActor(*(collider->config->actor));
        //collider->enableCollisionsAndQueries(true);
        rigidbody->setNormalGravity(prevGravity);
        collider->config->actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
    }

    TMsgNoClipToggle msgNoClip;
    me->sendMsg(msgNoClip);
}

void TCompNoClipController::onMsgScenePaused(const TMsgScenePaused & msg)
{
    paused = msg.isPaused;
}

void TCompNoClipController::update(float dt)
{
    if (!paused && isInNoClipMode) {

        VEC2 movementValue = VEC2::Zero;
        float player_accel = speed * dt;

        if (EngineInput["btUp"].isPressed()
            || EngineInput["btDown"].isPressed()
            || EngineInput["btLeft"].isPressed()
            || EngineInput["btRight"].isPressed()) {

            movementValue.x = EngineInput["btRight"].value - EngineInput["btLeft"].value;
            movementValue.y = EngineInput["btUp"].value - EngineInput["btDown"].value;

            float total_value = movementValue.Length();
        }
        else {
            player_accel = 0.f;
        }


        float yaw, pitch, roll;
        CEntity *player_camera = getEntityByName("TPCamera");
        TCompTransform *c_my_transform = get<TCompTransform>();
        TCompTransform * trans_camera = player_camera->get<TCompTransform>();
        c_my_transform->getYawPitchRoll(&yaw, &pitch, &roll);

        VEC3 up = trans_camera->getFront();
        VEC3 normal_norm = c_my_transform->getUp();
        VEC3 proj = projectVector(up, normal_norm);
        VEC3 dir = getMotionDir(proj, normal_norm.Cross(-proj), movementValue);

        if (EngineInput["btSecAction"].isPressed()) {

            /* Space to go up */
            dir += VEC3(0, 1, 0);
            dir.Normalize();
            player_accel = speed * dt;
        }
        else if (EngineInput["btSlow"].isPressed()) {
            
            /* CTRL to go down */
            dir += VEC3(0, -1, 0);
            dir.Normalize();
            player_accel = speed * dt;
        }

        if (EngineInput["btRun"].isPressed()) {
            player_accel *= 2.f;
        }

        if (dir == VEC3::Zero) dir = proj;

        float dir_yaw = getYawFromVector(dir);
        Quaternion my_rotation = c_my_transform->getRotation();
        Quaternion new_rotation = Quaternion::CreateFromYawPitchRoll(dir_yaw, pitch, 0);
        Quaternion quat = Quaternion::Lerp(my_rotation, new_rotation, rotationSpeed * dt);
        c_my_transform->setRotation(quat);
        TCompRigidbody *rigidbody = get<TCompRigidbody>();
        rigidbody->setGlobalPose(c_my_transform->getPosition() + dir * player_accel, c_my_transform->getRotation());
        c_my_transform->setPosition(c_my_transform->getPosition() + dir * player_accel);
    }
}

VEC3 TCompNoClipController::getMotionDir(const VEC3 & front, const VEC3 & left, const VEC2& movementValue)
{
    VEC3 dir = VEC3::Zero;

    dir += movementValue.y * front;

    dir += movementValue.x * left;

    dir.Normalize();

    return dir;
}

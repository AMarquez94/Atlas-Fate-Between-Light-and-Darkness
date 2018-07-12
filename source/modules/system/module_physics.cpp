#include "mcv_platform.h"
#include "module_physics.h"
#include "entity/entity.h"
#include "components/comp_transform.h"
#include "render/mesh/mesh.h"
#include "render/mesh/mesh_loader.h"
#include "physics/physics_collider.h"
#include "components/physics/comp_rigidbody.h"
#include "components/comp_tags.h"

#pragma comment(lib,"PhysX3_x64.lib")
#pragma comment(lib,"PhysX3Common_x64.lib")
#pragma comment(lib,"PhysX3Extensions.lib")
#pragma comment(lib,"PxFoundation_x64.lib")
#pragma comment(lib,"PxPvdSDK_x64.lib")
#pragma comment(lib,"PhysX3CharacterKinematic_x64.lib")
#pragma comment(lib,"PhysX3Cooking_x64.lib")

using namespace physx;

const VEC3 CModulePhysics::gravity(0, -1, 0);
const float CModulePhysics::gravityMod = 9.8f;
physx::PxQueryFilterData CModulePhysics::defaultFilter;

bool CModulePhysics::start()
{
    gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    assert(gFoundation != nullptr);

    if (!gFoundation) {
        return false;
    }

    gPvd = PxCreatePvd(*gFoundation);

    if (!gPvd) {
        return false;
    }

    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    bool  is_ok = gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
    gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));
    gDispatcher = PxDefaultCpuDispatcherCreate(2);

    if (!gPhysics)
        fatal("PxCreatePhysics failed");

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0, -9.8f, 0);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = CustomFilterShader;
    sceneDesc.flags = PxSceneFlag::eENABLE_KINEMATIC_PAIRS | PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS | PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    gScene = gPhysics->createScene(sceneDesc);

    PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();

    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    CPhysicsCollider::default_material = gPhysics->createMaterial(0.5f, 0.5f, 0.2f);
    mControllerManager = PxCreateControllerManager(*gScene);
    gScene->setSimulationEventCallback(&customSimulationEventCallback);
    PxInitExtensions(*gPhysics, gPvd);

    // Set a default filter to do query checks
    physx::PxFilterData pxFilterData;
    pxFilterData.word0 = FilterGroup::Scenario;
    defaultFilter.data = pxFilterData;

    return true;
}

void CModulePhysics::update(float delta)
{
    if (!gScene)
        return;

    gScene->simulate(delta);
    gScene->fetchResults(true);

    PxU32 nbActorsOut = 0;
    PxActor**actors = gScene->getActiveActors(nbActorsOut);

    for (unsigned int i = 0; i < nbActorsOut; ++i) {

        if (actors[i]->is<PxRigidActor>())
        {
            PxRigidActor* rigidActor = ((PxRigidActor*)actors[i]);
            PxTransform PxTrans = rigidActor->getGlobalPose();
            PxVec3 pxpos = PxTrans.p;
            PxQuat pxq = PxTrans.q;
            CHandle h_comp_collider;
            h_comp_collider.fromVoidPtr(rigidActor->userData);

            CEntity* e = h_comp_collider.getOwner();
            TCompCollider* compCollider = h_comp_collider;
            if (!h_comp_collider.isValid() || !h_comp_collider.getOwner().isValid()) {
                continue;
            }
            TCompTransform * compTransform = e->get<TCompTransform>();
            TCompRigidbody * compRigibody = e->get<TCompRigidbody>();

            if (compRigibody)
            {
                if (!compRigibody->is_controller)
                {
                    compTransform->setRotation(QUAT(pxq.x, pxq.y, pxq.z, pxq.w));
                    compTransform->setPosition(VEC3(pxpos.x, pxpos.y, pxpos.z));
                    compRigibody->lastFramePosition = VEC3(pxpos.x, pxpos.y, pxpos.z);
                }
            }
        }
    }
}

void CModulePhysics::render()
{

}

void CModulePhysics::CustomSimulationEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
    for (PxU32 i = 0; i < count; ++i)
    {

        CHandle h_trigger_comp_collider;
        h_trigger_comp_collider.fromVoidPtr(pairs[i].triggerActor->userData);

        CHandle h_other_comp_collider;
        h_other_comp_collider.fromVoidPtr(pairs[i].otherActor->userData);

        if (!h_other_comp_collider.isValid() || !h_trigger_comp_collider.isValid() || !h_trigger_comp_collider.getOwner().isValid()) {
            continue;
        }

        CEntity* e_trigger = h_trigger_comp_collider.getOwner();

        if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
        {
            e_trigger->sendMsg(TMsgTriggerExit{ h_other_comp_collider.getOwner() });
            TCompCollider * comp = (TCompCollider*)h_trigger_comp_collider;
            TCompCollider * comp_enemy = (TCompCollider*)h_other_comp_collider;
            continue;
        }

        //dbg("trigger touch\n");
        if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            e_trigger->sendMsg(TMsgTriggerEnter{ h_other_comp_collider.getOwner() });
            TCompCollider * comp = (TCompCollider*)h_trigger_comp_collider;
            TCompCollider * comp_enemy = (TCompCollider*)h_other_comp_collider;
        }
        else if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            e_trigger->sendMsg(TMsgTriggerExit{ h_other_comp_collider.getOwner() });
            TCompCollider * comp = (TCompCollider*)h_trigger_comp_collider;
            TCompCollider * comp_enemy = (TCompCollider*)h_other_comp_collider;
        }
    }
}

void CModulePhysics::CustomSimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    for (PxU32 i = 0; i < nbPairs; i++)
    {
        const PxContactPair& cp = pairs[i];

				CHandle h_actor_1;
				h_actor_1.fromVoidPtr(pairHeader.actors[0]->userData);

				CHandle h_actor_2;
				h_actor_2.fromVoidPtr(pairHeader.actors[1]->userData);

				if (h_actor_1.isValid() && h_actor_1.getOwner().isValid() && h_actor_2.isValid() && h_actor_2.getOwner().isValid()) {

					CEntity* entity1 = h_actor_1.getOwner();
					CEntity* entity2 = h_actor_2.getOwner();

					if (cp.events & (PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS)) {

                        /* Only manages contact between rigidbodies */
						TCompRigidbody * rigidbody1 = entity1->get<TCompRigidbody>();
						TCompRigidbody * rigidbody2 = entity2->get<TCompRigidbody>();

						if (rigidbody1 && rigidbody2) {
							TMsgPhysxContact msg;
							msg.other_entity = h_actor_2.getOwner();
							entity1->sendMsg(msg);

							msg.other_entity = h_actor_1.getOwner();
							entity2->sendMsg(msg);
						}
					}
					else {
						/* TODO: To be implemented */
					}
				}
        //dbg("contact found\n");
    }
}

/* Auxiliar physics methods */

bool CModulePhysics::Raycast(const VEC3 & origin, const VEC3 & dir, float distance, physx::PxRaycastHit & hit, physx::PxQueryFlags flag, physx::PxQueryFilterData filterdata)
{
    PxVec3 px_origin = PxVec3(origin.x, origin.y, origin.z);
    PxVec3 px_dir = PxVec3(dir.x, dir.y, dir.z); // [in] Normalized ray direction
    PxReal px_distance = (PxReal)(distance); // [in] Raycast max distance

    PxRaycastBuffer px_hit; // [out] Raycast results
    filterdata.flags = flag;

    bool status = gScene->raycast(px_origin, px_dir, px_distance, px_hit, PxHitFlags(PxHitFlag::eDEFAULT), filterdata); // Closest hit
    hit = px_hit.block;

    return status;
}

/* Returns true if there was some hit with the sweep cast. Hit will contain all hits */
bool CModulePhysics::Sweep(physx::PxGeometry& geometry, const VEC3 & position, const QUAT & rotation, const VEC3 & direction, float distance, std::vector<physx::PxSweepHit>& hits, physx::PxQueryFlags flag, physx::PxQueryFilterData filterdata)
{
    PxSweepHit sweepHit[256];     //With 256 it is supossed to be enough
    PxSweepBuffer px_hit(sweepHit, 256);
    filterdata.flags = flag;
    PxVec3 px_dir = PxVec3(direction.x, direction.y, direction.z); // [in] Normalized sweep direction


    physx::PxTransform transform(PxVec3(position.x, position.y, position.z), PxQuat(rotation.x, rotation.y, rotation.z, rotation.w));
    bool status = gScene->sweep(geometry, transform, px_dir, distance, px_hit, PxHitFlags(PxHitFlag::eDEFAULT), filterdata);

    if (status) {
        for (PxU32 i = 0; i < px_hit.nbTouches; i++) {
            hits.push_back(px_hit.touches[i]);
        }
    }

    return status;
}

/* Returns true if there was some hit with the sphere cast. Hit will contain all hits */
bool CModulePhysics::Overlap(physx::PxGeometry& geometry, VEC3 pos, std::vector<physx::PxOverlapHit> & hits, physx::PxQueryFilterData filterdata)
{
    PxOverlapHit overlapHit[256];     //With 256 it is supossed to be enough
    PxOverlapBuffer px_hit(overlapHit, 256);

    physx::PxTransform transform(PxVec3(pos.x, pos.y, pos.z));

    bool status = gScene->overlap(geometry, transform, px_hit, filterdata);

    if (status) {
        for (PxU32 i = 0; i < px_hit.nbTouches; i++) {
            hits.push_back(px_hit.touches[i]);
        }
    }

    return status;
}

PxFixedJoint* CModulePhysics::CreateFixedJoint(physx::PxRigidActor * dynamicActor, const physx::PxTransform & dynamicActorTransform, physx::PxRigidActor * otherActor, const physx::PxTransform & otherActorTransform)
{
  /* TODO: not tested */
  physx::PxPhysics* physxFactory = getPhysxFactory();
  physx::PxFixedJoint* joint = physx::PxFixedJointCreate(*physxFactory, dynamicActor, dynamicActorTransform, otherActor, otherActorTransform);
  joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
  return joint;
}

physx::PxDistanceJoint * CModulePhysics::CreateDistanceJoint(physx::PxRigidActor * dynamicActor, const physx::PxTransform & dynamicActorTransform, physx::PxRigidActor * otherActor, const physx::PxTransform & otherActorTransform)
{
  /* TODO: not tested */
  physx::PxPhysics* physxFactory = getPhysxFactory();
  physx::PxDistanceJoint* joint = physx::PxDistanceJointCreate(*physxFactory, dynamicActor, dynamicActorTransform, otherActor, otherActorTransform);
  joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
  return joint;
}

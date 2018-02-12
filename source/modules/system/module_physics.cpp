#include "mcv_platform.h"
#include "module_physics.h"
#include "entity/entity.h"
#include "components/comp_transform.h"


#pragma comment(lib,"PhysX3_x64.lib")
#pragma comment(lib,"PhysX3Common_x64.lib")
#pragma comment(lib,"PhysX3Extensions.lib")
#pragma comment(lib,"PxFoundation_x64.lib")
#pragma comment(lib,"PxPvdSDK_x64.lib")
#pragma comment(lib, "PhysX3CharacterKinematic_x64.lib")

using namespace physx;

PxReal stackZ = 10.0f;

std::vector<PxActor*> actors;
void CModulePhysics::createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
    PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
    for ( PxU32 i = 0; i<size;i++ )
    {
        for ( PxU32 j = 0;j<size - i;j++ )
        {
            PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
            PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
            body->attachShape(*shape);
            PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
            gScene->addActor(*body);
            actors.push_back(body);
        }
    }
    shape->release();
}

bool CModulePhysics::start()
{
    gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    assert(gFoundation != nullptr );
    
    if ( !gFoundation ) {
        return false;
    }

    gPvd = PxCreatePvd(*gFoundation);
    if ( !gPvd ) {
        return false;
    }

    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    bool  is_ok = gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);

    PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
    if ( pvdClient )
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
    gScene->addActor(*groundPlane);

    for ( PxU32 i = 0;i<1;i++ )
        createStack(PxTransform(PxVec3(0, 0, stackZ -= 10.0f)), 10, 2.0f);

    mControllerManager = PxCreateControllerManager(*gScene);


    PxControllerDesc* cDesc;
    PxBoxControllerDesc boxDesc;
    PxCapsuleControllerDesc capsuleDesc;

    PX_ASSERT(desc.mType == PxControllerShapeType::eCAPSULE);
    capsuleDesc.height = 1.f;
    capsuleDesc.radius = 0.5f;
    capsuleDesc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
    cDesc = &capsuleDesc;

    cDesc->material = gMaterial;

   ctrl = static_cast<PxCapsuleController*>(mControllerManager->createController(*cDesc));
    PX_ASSERT(ctrl);

  return true;
}

void CModulePhysics::update(float delta)
{

    const Input::TButton& bt = CEngine::get().getInput().host(Input::PLAYER_1).keyboard().key(VK_SPACE);
    if ( bt.isPressed() )
    {
        for(auto actor : actors) {
            ((PxRigidBody*)actor)->addForce(PxVec3(0.f, 1000.f, 0.f), PxForceMode::eACCELERATION);
        }
       /* VEC3 delta_move = VEC3(0.f,0.f, -5.f*delta);

        delta_move += VEC3(0.f, -9.81f*delta, 0.f);
        ctrl->move(physx::PxVec3(delta_move.x, delta_move.y, delta_move.z), 0.0f, delta, PxControllerFilters());*/
    }

    if ( !gScene )
        return;
    gScene->simulate(delta);
    gScene->fetchResults(true);
}

void CModulePhysics::render()
{
}

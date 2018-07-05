#include "mcv_platform.h"
#include "physics_collider.h"
#include "components/comp_transform.h"
#include "physics/physics_mesh.h"
#include "render/mesh/mesh_loader.h"

physx::PxMaterial* CPhysicsCollider::default_material;

void CPhysicsCollider::setupFiltering(physx::PxShape* shape, physx::PxU32 filterGroup, physx::PxU32 filterMask)
{
    physx::PxFilterData filterData;
    filterData.word0 = filterGroup; // word0 = own ID
    filterData.word1 = filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;
    shape->setSimulationFilterData(filterData);
    shape->setQueryFilterData(filterData);
}

void CPhysicsCollider::setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask)
{
    physx::PxFilterData filterData;
    filterData.word0 = filterGroup; // word0 = own ID
    filterData.word1 = filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;
    const physx::PxU32 numShapes = actor->getNbShapes();
    std::vector<physx::PxShape*> shapes;
    shapes.resize(numShapes);
    actor->getShapes(&shapes[0], numShapes);

    for (physx::PxU32 i = 0; i < numShapes; i++)
    {
        physx::PxShape* shape = shapes[i];
        shape->setSimulationFilterData(filterData);
        shape->setQueryFilterData(filterData);
    }
}

void CPhysicsCollider::setAsTrigger(physx::PxShape * shape, bool state)
{
    shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, !state);
    shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !state);
    shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, state);
}

void CPhysicsCollider::createStatic(physx::PxShape* actor_shape, TCompTransform * c_transform)
{
    shape = actor_shape;
    physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();
    physx::PxScene * gScene = EnginePhysics.getPhysxScene();

    VEC3 pos = c_transform->getPosition();
    QUAT quat = c_transform->getRotation();
    physx::PxTransform transform(physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(quat.x, quat.y, quat.z, quat.w));
    physx::PxRigidActor *rigid_actor = gPhysics->createRigidStatic(transform);

    actor = rigid_actor;
    actor->attachShape(*actor_shape);
    actor_shape->release();

    setupFiltering(actor, group, mask);
    gScene->addActor(*actor);
}

void CPhysicsCollider::createDynamic(physx::PxShape* actor_shape, TCompTransform * c_transform, float mass)
{
    shape = actor_shape;
    physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();
    physx::PxScene * gScene = EnginePhysics.getPhysxScene();

    VEC3 pos = c_transform->getPosition();
    QUAT quat = c_transform->getRotation();
    physx::PxTransform transform(physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(quat.x, quat.y, quat.z, quat.w));
    physx::PxRigidDynamic* rigid_actor = gPhysics->createRigidDynamic(transform);
    //rigid_actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, is_kinematic);

    //physx::PxRigidBodyExt::updateMassAndInertia(*rigid_actor, 0);
    //rigid_actor->mass
    rigid_actor->setMass(mass);
    rigid_actor->setMassSpaceInertiaTensor(physx::PxVec3(5, 5, 5));
    actor = rigid_actor;
    actor->attachShape(*actor_shape);
    actor_shape->release();

    setupFiltering(actor, group, mask);

    //physx::PxRigidBodyExt::updateMassAndInertia(*(physx::PxRigidDynamic*)actor, mass);
    gScene->addActor(*actor);
}

/* PHYSX BOX COLLIDER CONFIGURATION */
void CPhysicsBox::load(const json& j, TEntityParseContext& ctx) {

    assert(j.count("size"));

    VEC3 box_size = loadVEC3(j["size"]);
    size = physx::PxVec3(box_size.x, box_size.y, box_size.z);
    step_offset = j.value("step_offset", 0.2f);
    contact_offset = j.value("contact_offset", contact_offset);
}

physx::PxShape* CPhysicsBox::createShape() {

    material = default_material;
    physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();

    shape = gPhysics->createShape(physx::PxBoxGeometry(size.x, size.y, size.z), *material);
	shape->setLocalPose(physx::PxTransform(physx::PxVec3(center.x, center.y + size.y, center.z)));
	shape->setContactOffset(contact_offset);
    setupFiltering(shape, group, mask);
    setAsTrigger(shape, is_trigger);

    return shape;
}

physx::PxController* CPhysicsBox::createController(TCompTransform * c_transform) {

    material = EnginePhysics.getPhysxFactory()->createMaterial(0.6f, 0.4f, 0.1f);
    physx::PxControllerDesc* cDesc;
    physx::PxBoxControllerDesc boxDesc;
    physx::PxControllerManager * gManager = EnginePhysics.getPhysxController();

    boxDesc.halfHeight = size.y;
    boxDesc.halfSideExtent = size.x;
    boxDesc.halfForwardExtent = size.z;
    cDesc = &boxDesc;
    cDesc->material = material;
    cDesc->contactOffset = contact_offset;
    cDesc->stepOffset = step_offset;
    physx::PxController* controller = gManager->createController(*cDesc);
    physx::PxBoxController * ctrl = static_cast<physx::PxBoxController*>(controller);
    PX_ASSERT(ctrl);

    VEC3 pos = c_transform->getPosition();
    QUAT quat = c_transform->getRotation();
    ctrl->setFootPosition(physx::PxExtendedVec3(pos.x, pos.y, pos.z));
    ctrl->setContactOffset(contact_offset);
    ctrl->setStepOffset(step_offset);

    actor = ctrl->getActor();
    const physx::PxU32 numShapes = actor->getNbShapes();
    std::vector<physx::PxShape*> shapes;
    shapes.resize(numShapes);
    actor->getShapes(&shapes[0], numShapes);
    shape = shapes[0];
    actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
    setupFiltering(actor, group, mask);
    return ctrl;
}

void CPhysicsBox::debugInMenu() {

}

/* PHYSX BOX COLLIDER CONFIGURATION */
void CPhysicsPlane::load(const json& j, TEntityParseContext& ctx) {

}

physx::PxShape* CPhysicsPlane::createShape() {

    material = default_material;
    physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();

    shape = gPhysics->createShape(physx::PxPlaneGeometry(), *material);
	shape->setLocalPose(physx::PxTransform(physx::PxVec3(center.x, center.y, center.z)));
    setupFiltering(shape, group, mask);
    setAsTrigger(shape, is_trigger);

    return shape;
}

void CPhysicsPlane::debugInMenu() {

}

void CPhysicsSphere::load(const json& j, TEntityParseContext& ctx) {

    radius = j.value("radius", 0.f);
}

physx::PxShape* CPhysicsSphere::createShape() {

    material = default_material;
    physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();

    shape = gPhysics->createShape(physx::PxSphereGeometry(radius), *material);
	shape->setLocalPose(physx::PxTransform(physx::PxVec3(center.x, center.y, center.z)));
	shape->setContactOffset(contact_offset);
    setupFiltering(shape, group, mask);
    setAsTrigger(shape, is_trigger);

    return shape;
}

void CPhysicsSphere::debugInMenu() {

}

void CPhysicsCapsule::load(const json& j, TEntityParseContext& ctx) {

    height = j.value("height", 0.f);
    radius = j.value("radius", 0.f);
    step_offset = j.value("step_offset", 0.2f);
}

physx::PxShape* CPhysicsCapsule::createShape() {

    material = default_material;
    physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();

    shape = gPhysics->createShape(physx::PxCapsuleGeometry(radius, height), *material);
	shape->setLocalPose(physx::PxTransform(physx::PxVec3(center.x, center.y, center.z)));
	shape->setContactOffset(contact_offset);
    setupFiltering(shape, group, mask);
    setAsTrigger(shape, is_trigger);

    return shape;
}

physx::PxController* CPhysicsCapsule::createController(TCompTransform * c_transform) {

    material = EnginePhysics.getPhysxFactory()->createMaterial(0.6f, 0.4f, 0.1f);
    physx::PxControllerDesc* cDesc;
    physx::PxCapsuleControllerDesc capsuleDesc;
    physx::PxControllerManager * gManager = EnginePhysics.getPhysxController();

    capsuleDesc.height = height;
    capsuleDesc.radius = radius;
    capsuleDesc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
    capsuleDesc.scaleCoeff = 0.99f;
    cDesc = &capsuleDesc;
    cDesc->material = material;
    cDesc->contactOffset = contact_offset;
    physx::PxCapsuleController * ctrl = static_cast<physx::PxCapsuleController*>(gManager->createController(*cDesc));
    PX_ASSERT(ctrl);

    VEC3 pos = c_transform->getPosition();
    QUAT quat = c_transform->getRotation();
    ctrl->setFootPosition(physx::PxExtendedVec3(pos.x, pos.y, pos.z));
    ctrl->setContactOffset(contact_offset);
    ctrl->setStepOffset(step_offset);

    actor = ctrl->getActor();
    const physx::PxU32 numShapes = actor->getNbShapes();
    std::vector<physx::PxShape*> shapes;
    shapes.resize(numShapes);
    actor->getShapes(&shapes[0], numShapes);
    shape = shapes[0];
    actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
    setupFiltering(actor, group, mask);

    return ctrl;
}

void CPhysicsCapsule::debugInMenu() {

}

void CPhysicsConvex::load(const json& j, TEntityParseContext& ctx) {

    filename = j.value("name", "");
}

physx::PxShape* CPhysicsConvex::createShape() {

    material = default_material;
    mesh = loadPhysicsMesh(filename.c_str());
    TMeshLoader * collider_mesh = loadCollider(filename.c_str());

    physx::PxCooking * gCooking = EnginePhysics.getCooking();
    physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();
    physx::PxScene * gScene = EnginePhysics.getPhysxScene();

    physx::PxCookingParams params = gCooking->getParams();
    params.convexMeshCookingType = convextype;
    params.gaussMapLimit = 256;
    gCooking->setParams(params);

    // Setup the convex mesh descriptor
    physx::PxConvexMeshDesc desc;
    desc.points.data = collider_mesh->vtxs.data();
    desc.points.count = collider_mesh->header.num_vertexs;
    desc.points.stride = sizeof(physx::PxVec3);
    desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

    physx::PxConvexMesh* convex = gCooking->createConvexMesh(desc, gPhysics->getPhysicsInsertionCallback());
    physx::PxConvexMeshGeometry convex_geo = physx::PxConvexMeshGeometry(convex, physx::PxMeshScale(), physx::PxConvexMeshGeometryFlags());
	shape = gPhysics->createShape(convex_geo, *material);
    setupFiltering(shape, group, mask);
    setAsTrigger(shape, is_trigger);

    return shape;
}

void CPhysicsConvex::debugInMenu() {

}

void CPhysicsTriangleMesh::load(const json& j, TEntityParseContext& ctx) {

    filename = j.value("name", "");
}

physx::PxShape* CPhysicsTriangleMesh::createShape() {

    material = default_material;
    mesh = loadPhysicsMesh(filename.c_str());
    TMeshLoader * collider_mesh = loadCollider(filename.c_str());

    physx::PxCooking * gCooking = EnginePhysics.getCooking();
    physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();
    physx::PxScene * gScene = EnginePhysics.getPhysxScene();

    // Setup the triangle mesh descriptor
    physx::PxTriangleMeshDesc meshDesc;
    meshDesc.points.data = collider_mesh->vtxs.data();
    meshDesc.points.count = collider_mesh->header.num_vertexs;
    meshDesc.points.stride = collider_mesh->header.bytes_per_vtx;
    meshDesc.flags = physx::PxMeshFlag::e16_BIT_INDICES | physx::PxMeshFlag::eFLIPNORMALS;

    meshDesc.triangles.data = collider_mesh->idxs.data();
    meshDesc.triangles.count = collider_mesh->header.num_indices / 3;
    meshDesc.triangles.stride = 3 * collider_mesh->header.bytes_per_idx;

    physx::PxTriangleMesh * tri_mesh = gCooking->createTriangleMesh(meshDesc, gPhysics->getPhysicsInsertionCallback());
    physx::PxTriangleMeshGeometry tri_geo = physx::PxTriangleMeshGeometry(tri_mesh, physx::PxMeshScale());
	shape = gPhysics->createShape(tri_geo, *material);
    setupFiltering(shape, group, mask);
    setAsTrigger(shape, is_trigger);

    return shape;
}

void CPhysicsTriangleMesh::debugInMenu() {

}
#include "mcv_platform.h"
#include "collider_resource.h"
#include "components/comp_transform.h"
// ----------------------------------------------
class ColliderMeshResourceClass : public CResourceClass {
public:
	ColliderMeshResourceClass() {
		class_name = "colliders";
		extensions = { ".collider" };
	}
	IResource* create(const std::string& name) const override {
		dbg("Creating mesh %s\n", name.c_str());
		CPhysicsMesh* res = NULL;// loadCollider(name.c_str());
		return res;
	}
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<CPhysicsCollider>() {
	static ColliderMeshResourceClass the_resource_class;
	return &the_resource_class;
}

bool CPhysicsMesh::create(
	const void* vertex_data,
	const void* index_data,
	uint32_t vertex_size,
	uint32_t index_size){

	assert(vertex_data != nullptr);
	assert(index_data != nullptr);

	num_vertexs = vertex_size;
	num_indices = index_size;

	uint8_t *vertex_buffer = (uint8_t*)vertex_data;
	uint8_t *index_buffer = (uint8_t*)index_data;

	vtxs = std::vector<uint8_t>(vertex_buffer, vertex_buffer + vertex_size);
	idxs = std::vector<uint8_t>(index_buffer, index_buffer + index_size);
}

void CPhysicsMesh::destroy()
{
	// Release data
}

void CPhysicsMesh::debugInMenu() {

	ImGui::Text("%d vertexs", num_vertexs);
	ImGui::Text("%d vertexs", num_indices);
}

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

/* PHYSX BOX COLLIDER CONFIGURATION */
void CPhysicsBox::load(const json& j, TEntityParseContext& ctx){

	assert(j.count("size"));

	VEC3 box_size = loadVEC3(j["size"]);
	size = physx::PxVec3(box_size.x, box_size.y, box_size.z);
}

void CPhysicsBox::create(TCompTransform * c_transform){

	physx::PxRigidActor * rigidactor;
	physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();
	physx::PxScene * gScene = EnginePhysics.getPhysxScene();
	physx::PxShape * actor_shape = gPhysics->createShape(physx::PxBoxGeometry(size.x, size.y, size.z), *material);
	actor_shape->setLocalPose(physx::PxTransform(physx::PxVec3(center.x, center.y, center.z)));
	actor_shape->setContactOffset(contact_offset);
	setupFiltering(actor_shape, group, mask);
	setAsTrigger(actor_shape, is_trigger);

	VEC3 pos = c_transform->getPosition();
	QUAT quat = c_transform->getRotation();
	physx::PxTransform initialTrans(physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(quat.x, quat.y, quat.z, quat.w));
	actor = gPhysics->createRigidStatic(initialTrans);

	assert(actor);
	assert(actor_shape);

	rigidactor->attachShape(*actor_shape);
	actor_shape->release();
	gScene->addActor(*actor);
}

void CPhysicsBox::debugInMenu(){

}

/* PHYSX BOX COLLIDER CONFIGURATION */
void CPhysicsPlane::load(const json& j, TEntityParseContext& ctx) {

}

void CPhysicsPlane::create(TCompTransform * c_transform) {

	physx::PxRigidActor * rigidactor;
	physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();
	physx::PxScene * gScene = EnginePhysics.getPhysxScene();

	VEC3 pos = c_transform->getPosition();
	QUAT quat = c_transform->getRotation();
	actor = physx::PxCreatePlane(*gPhysics, physx::PxPlane(pos.x, pos.y, pos.z, 0), *material);
	setupFiltering(actor, group, mask);

	assert(actor);
	gScene->addActor(*actor);
}

void CPhysicsPlane::debugInMenu() {

}


void CPhysicsSphere::load(const json& j, TEntityParseContext& ctx){

	radius = j.value("radius", 0.f);
}

void CPhysicsSphere::create(TCompTransform * c_transform){

	physx::PxRigidActor * rigidactor;
	physx::PxPhysics * gPhysics = EnginePhysics.getPhysxFactory();
	physx::PxScene * gScene = EnginePhysics.getPhysxScene();
	physx::PxShape * actor_shape = gPhysics->createShape(physx::PxSphereGeometry(radius), *material);
	actor_shape->setLocalPose(physx::PxTransform(physx::PxVec3(center.x, center.y, center.z)));
	actor_shape->setContactOffset(contact_offset);
	setupFiltering(actor_shape, group, mask);
	setAsTrigger(actor_shape, is_trigger);

	VEC3 pos = c_transform->getPosition();
	QUAT quat = c_transform->getRotation();
	physx::PxTransform initialTrans(physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(quat.x, quat.y, quat.z, quat.w));
	actor = gPhysics->createRigidStatic(initialTrans);

	assert(actor);
	assert(actor_shape);

	rigidactor->attachShape(*actor_shape);
	actor_shape->release();
	gScene->addActor(*actor);
}

void CPhysicsSphere::debugInMenu(){

}

void CPhysicsCapsule::load(const json& j, TEntityParseContext& ctx) {

}

void CPhysicsCapsule::create(TCompTransform * c_transform) {

	physx::PxControllerDesc* cDesc;
	physx::PxCapsuleControllerDesc capsuleDesc;
	PX_ASSERT(desc.mType == physx::PxControllerShapeType::eCAPSULE);

	capsuleDesc.height = height;
	capsuleDesc.radius = radius;
	capsuleDesc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
	cDesc = &capsuleDesc;
	cDesc->material = material;
	cDesc->contactOffset = 0.01f;
	physx::PxCapsuleController * ctrl = static_cast<physx::PxCapsuleController*>(mControllerManager->createController(*cDesc));
	PX_ASSERT(ctrl);

	ctrl->setFootPosition(physx::PxExtendedVec3(pos.x, pos.y, pos.z));
	ctrl->setContactOffset(0.01f);
	actor = ctrl->getActor();
	setupFiltering(actor, config.group, config.mask);
}

void CPhysicsCapsule::debugInMenu() {

}

void CPhysicsConvex::load(const json& j, TEntityParseContext& ctx) {

}

void CPhysicsConvex::create(TCompTransform * c_transform) {

	PxCookingParams params = gCooking->getParams();
	params.convexMeshCookingType = PxConvexMeshCookingType::eQUICKHULL;
	params.gaussMapLimit = 256;
	gCooking->setParams(params);

	// Setup the convex mesh descriptor
	PxConvexMeshDesc desc;
	desc.points.data = collider_mesh->vtxs.data();
	desc.points.count = collider_mesh->header.num_vertexs;
	desc.points.stride = sizeof(PxVec3);
	desc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

	PxConvexMesh* convex = gCooking->createConvexMesh(desc, gPhysics->getPhysicsInsertionCallback());
	PxConvexMeshGeometry convex_geo = PxConvexMeshGeometry(convex, PxMeshScale(), PxConvexMeshGeometryFlags());
	actor = gPhysics->createRigidStatic(initialTrans);
	shape = actor->createShape(convex_geo, *gMaterial);

	if (config.is_trigger)
	{
		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	}

	actor->attachShape(*shape);
	shape->release();
	gScene->addActor(*actor);

	setupFiltering(actor, config.group, config.mask);
	comp_collider.actor = actor;
	actor->userData = h_comp_collider.asVoidPtr();
}

void CPhysicsConvex::debugInMenu() {

}

void CPhysicsConvex::load(const json& j, TEntityParseContext& ctx) {

}

void CPhysicsConvex::create(TCompTransform * c_transform) {

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.data = collider_mesh->vtxs.data();
	meshDesc.points.count = collider_mesh->header.num_vertexs;
	meshDesc.points.stride = collider_mesh->header.bytes_per_vtx;
	meshDesc.flags = PxMeshFlag::e16_BIT_INDICES | PxMeshFlag::eFLIPNORMALS;

	meshDesc.triangles.data = collider_mesh->idxs.data();
	meshDesc.triangles.count = collider_mesh->header.num_indices / 3;
	meshDesc.triangles.stride = 3 * collider_mesh->header.bytes_per_idx;

	PxTriangleMesh * tri_mesh = gCooking->createTriangleMesh(meshDesc, gPhysics->getPhysicsInsertionCallback());
	PxTriangleMeshGeometry tri_geo = PxTriangleMeshGeometry(tri_mesh, PxMeshScale());
	actor = gPhysics->createRigidStatic(initialTrans);
	shape = actor->createShape(tri_geo, *gMaterial);

	if (config.is_trigger)
	{
		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	}

	actor->attachShape(*shape);
	shape->release();
	gScene->addActor(*actor);

	setupFiltering(actor, config.group, config.mask);
	comp_collider.actor = actor;
	actor->userData = h_comp_collider.asVoidPtr();
	return;
}

void CPhysicsConvex::debugInMenu() {

}
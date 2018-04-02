#include "mcv_platform.h"
#include "comp_rigidbody.h"
#include "comp_collider.h"
#include "../comp_transform.h"
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

	TCompCollider * c_collider = get<TCompCollider>();
	velocity = physx::PxVec3(0,0,0);

	if (CHandle(this).getOwner().isValid()) {
		TCompTransform *transform = get<TCompTransform>();
		VEC3 new_pos = transform->getPosition();
		VEC3 delta_movement = new_pos - lastFramePosition;
		velocity = physx::PxVec3(delta_movement.x, delta_movement.y, delta_movement.z) / dt;
		lastFramePosition = new_pos;

		if (is_gravity) {
			if (is_grounded) totalDownForce = physx::PxVec3(0, 0, 0);
			physx::PxVec3 actualDownForce = physx::PxVec3(normal_gravity.x, normal_gravity.y, normal_gravity.z);
			velocity += (actualDownForce + totalDownForce);
			totalDownForce += 3.f * actualDownForce * dt;
		}
	}

	if (is_controller){
		physx::PxControllerCollisionFlags col = controller->move(velocity * dt, 0.f, dt, filters);
		is_grounded = col.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
	}
	else {
		TCompTransform *c_transform = get<TCompTransform>();
		VEC3 pos = c_transform->getPosition();
		QUAT quat = c_transform->getRotation();
		physx::PxTransform transform(physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(quat.x, quat.y, quat.z, quat.w));
		c_collider->config->actor->setGlobalPose(transform);
	}
}

/* Collider/Trigger messages */
void TCompRigidbody::registerMsgs() {
	DECL_MSG(TCompRigidbody, TMsgEntityCreated, onCreate);
}

void TCompRigidbody::onCreate(const TMsgEntityCreated& msg) {

	TCompCollider * c_collider = get<TCompCollider>();

	// Let the rigidbody handle the creation if it exists..
	if (c_collider != nullptr)
	{
		TCompTransform * compTransform = get<TCompTransform>();

		if (is_controller)
		{
			controller = c_collider->config->createController(compTransform);
			c_collider->config->actor->userData = CHandle(c_collider).asVoidPtr();
			c_collider->config->is_controller = true;
		}
		else
		{
			// Create the shape, the actor and set the user data
			physx::PxShape * shape = c_collider->config->createShape();
			c_collider->config->createDynamic(shape, compTransform);
			c_collider->config->actor->userData = CHandle(c_collider).asVoidPtr();
		}

		physx::PxFilterData * characterFilterData = new physx::PxFilterData();
		characterFilterData->word0 = c_collider->config->group;
		characterFilterData->word1 = c_collider->config->mask;

		filters = physx::PxControllerFilters();
		//filters.mFilterCallback = &customQueryFilter;
		filters.mFilterData = characterFilterData;
	}
}

void TCompRigidbody::Resize(float new_size)
{
	// Cannot resize if no collider is present.
	TCompCollider * c_collider = get<TCompCollider>();

	if (!c_collider || !c_collider->config->actor) 
		return;

	if(controller != NULL)
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

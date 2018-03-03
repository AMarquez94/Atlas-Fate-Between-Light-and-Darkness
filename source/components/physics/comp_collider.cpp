#include "mcv_platform.h"
#include "comp_collider.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_controller.h"

DECL_OBJ_MANAGER("collider", TCompCollider);

void TCompCollider::debugInMenu() {

}

void TCompCollider::load(const json& j, TEntityParseContext& ctx) {

	std::string shape = j["shape"].get<std::string>();
	if (strcmp("box", shape.c_str()) == 0)
	{
		config.shapeType = physx::PxGeometryType::eBOX;
	}
	else if (strcmp("sphere", shape.c_str()) == 0)
	{
		config.shapeType = physx::PxGeometryType::eSPHERE;
	}
	else if (strcmp("plane", shape.c_str()) == 0)
	{
		config.shapeType = physx::PxGeometryType::ePLANE;
	}
	else if (strcmp("capsule", shape.c_str()) == 0)
	{
		config.shapeType = physx::PxGeometryType::eCAPSULE;
	}
	else if (strcmp("convex", shape.c_str()) == 0)
	{
		config.shapeType = physx::PxGeometryType::eCONVEXMESH;
	}
	else if (strcmp("mesh", shape.c_str()) == 0)
	{
		config.shapeType = physx::PxGeometryType::eTRIANGLEMESH;
	}

	config.is_controller = j.value("is_controller", false);
	config.is_dynamic = j.value("is_dynamic", false);
	config.is_trigger = j.value("is_trigger", false);
	config.height = j.value("height", 0.f);
	config.currentHeight = config.height;
	config.radius = j.value("radius", 0.f);
	config.gravity = j.value("gravity", false);
	config.filename = j.value("name", "");

	config.group = CEngine::get().getPhysics().getFilterByName(j.value("group", "all"));
	config.mask = CEngine::get().getPhysics().getFilterByName(j.value("mask", "all"));

	if (j.count("halfExtent"))
		config.halfExtent = loadVEC3(j["halfExtent"]);

	// Seting flags for triggers, refactor in the future.a
	if (j.count("triggerKill"))
		config.flags = triggerKill;

	// Setting some default values.
	isInside = false;
	isGrounded = false;

}


void TCompCollider::registerMsgs() {

	DECL_MSG(TCompCollider, TMsgEntityCreated, onCreate);
	DECL_MSG(TCompCollider, TMsgTriggerEnter, onTriggerEnter);
	DECL_MSG(TCompCollider, TMsgTriggerExit, onTriggerExit);
	DECL_MSG(TCompCollider, TMsgEntityDestroyed, onDestroy);
}

void TCompCollider::onCreate(const TMsgEntityCreated& msg) {

	CEngine::get().getPhysics().createActor(*this);
	TCompTransform *transform = get<TCompTransform>();
	lastFramePosition = transform->getPosition();

	physx::PxFilterData * characterFilterData = new physx::PxFilterData();
	characterFilterData->word0 = config.group;
	characterFilterData->word1 = config.mask;

	filters = physx::PxControllerFilters();
	//filters.mFilterCallback = &customQueryFilter;
	filters.mFilterData = characterFilterData;
}

void TCompCollider::onDestroy(const TMsgEntityDestroyed & msg)
{
	if (actor && actor->getScene())
		actor->getScene()->removeActor(*actor);

	actor = nullptr;
	if (controller != NULL && controller) {
		controller->release();
		controller = nullptr;
	}
}

void TCompCollider::onTriggerEnter(const TMsgTriggerEnter& msg) {

	if (config.flags & triggerKill)
	{
		TMsgPlayerDead n_msg;
		n_msg.h_sender = CHandle(this).getOwner();
		CEntity * player = CHandle(msg.h_other_entity);// CHandle(msg.h_other_entity);
		player->sendMsg(n_msg);
	}
	dbg("Entered the trigger!!!!\n");
}

void TCompCollider::onTriggerExit(const TMsgTriggerExit& msg) {

	dbg("Exited the trigger!!!!\n");
}

void TCompCollider::update(float dt) {

	if (config.is_controller) {
		if (CHandle(this).getOwner().isValid()) {
			TCompTransform *transform = get<TCompTransform>();
			VEC3 new_pos = transform->getPosition();
			VEC3 delta_movement = new_pos - lastFramePosition;

			controller->move(physx::PxVec3(delta_movement.x, delta_movement.y, delta_movement.z), 0.f, dt, filters);
			lastFramePosition = new_pos;
		}
	}

	if (config.gravity) {

		if (isGrounded) {
			totalDownForce = physx::PxVec3(normal_gravity.x * dt, normal_gravity.y * dt, normal_gravity.z * dt);
		}
		else {
			totalDownForce += (physx::PxVec3(normal_gravity.x * dt, normal_gravity.y * dt, normal_gravity.z * dt)) * dt;
		}
		physx::PxControllerCollisionFlags col = controller->move(totalDownForce, 0.f, dt, filters);
		isGrounded = col.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN) ? true : false;
	}
}

void TCompCollider::Resize(float new_size)
{
	config.currentHeight = new_size;
	controller->resize((physx::PxReal)new_size);
}

void TCompCollider::SetUpVector(VEC3 new_up)
{
	controller->setUpDirection(physx::PxVec3(new_up.x, new_up.y, new_up.z));
}

VEC3 TCompCollider::GetUpVector()
{
	physx::PxVec3 upDirection = controller->getUpDirection();
	return VEC3(upDirection.x, upDirection.y, upDirection.z);
}

physx::PxQueryHitType::Enum TCompCollider::CustomQueryFilterCallback::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
{
	//const physx::PxFilterData& filterData1 = shape->getQueryFilterData();
	//CHandle n_comp_collider;
	//n_comp_collider.fromVoidPtr(actor->userData);
	//TCompCollider * n_collider = n_comp_collider;

	//if ((filterData.word0 & filterData1.word1) && (filterData1.word0 & filterData.word1))
	//{
	//	if (n_collider->config.flags & disableFilter) {

	//		dbg("blockea la collision\n");
	//		return physx::PxQueryHitType::eTOUCH;
	//	}
	//	return physx::PxQueryHitType::eBLOCK;
	//}
	return physx::PxQueryHitType::eTOUCH;
}
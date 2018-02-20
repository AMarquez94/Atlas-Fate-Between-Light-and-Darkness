#include "mcv_platform.h"
#include "comp_collider.h"
#include "../comp_transform.h"
#include "../player_controller/comp_player_controller.h"

DECL_OBJ_MANAGER("collider", TCompCollider);
VEC3 velocity = VEC3::Zero;

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

	config.is_character_controller = j.value("is_character_controller", false);
	config.is_dynamic = j.value("is_dynamic", false);
	config.is_trigger = j.value("is_trigger", false);
	config.height = j.value("height", 0.f);
	config.radius = j.value("radius", 0.f);
	config.gravity = j.value("gravity", false);

	// todo: extend this be able to parse more than group and mask
	config.group = CEngine::get().getPhysics().getFilterByName(j.value("group", "all"));
	config.mask = CEngine::get().getPhysics().getFilterByName(j.value("mask", "all"));

	if (j.count("halfExtent"))
		config.halfExtent = loadVEC3(j["halfExtent"]);
}

void TCompCollider::update(float dt) {

	if (config.is_character_controller) {

		TCompPlayerController *c_my_plyrcntlr = get<TCompPlayerController>();
		assert(c_my_plyrcntlr);
		VEC3 delta = c_my_plyrcntlr->delta_movement;
		controller->move(physx::PxVec3(delta.x, delta.y, delta.z), 0.f, dt, physx::PxControllerFilters());
		//velocity.x = delta.x;
		//velocity.z = delta.z;
	}

	if (config.gravity) {

		//velocity.y -= 9.81f * dt;
		controller->move(physx::PxVec3(normal_gravity.x, normal_gravity.y, normal_gravity.z), 0.f, dt, physx::PxControllerFilters());
	}
}

/* Collider/Trigger messages */
void TCompCollider::registerMsgs() {

	DECL_MSG(TCompCollider, TMsgEntityCreated, onCreate);
	DECL_MSG(TCompCollider, TMsgTriggerEnter, onTriggerEnter);
	DECL_MSG(TCompCollider, TMsgTriggerExit, onTriggerExit);
}

void TCompCollider::onCreate(const TMsgEntityCreated& msg) {

	CEngine::get().getPhysics().createActor(*this);
}

void TCompCollider::onTriggerEnter(const TMsgTriggerEnter& msg) {

	dbg("Entered the trigger!!!!\n");
}

void TCompCollider::onTriggerExit(const TMsgTriggerExit& msg) {

	dbg("Exited the trigger!!!!\n");
}

void TCompCollider::Resize(float new_size)
{
	controller->resize((physx::PxReal)new_size);
}

void TCompCollider::SetUpVector(VEC3 new_up)
{
	controller->setUpDirection(physx::PxVec3(new_up.x, new_up.y, new_up.z));
}
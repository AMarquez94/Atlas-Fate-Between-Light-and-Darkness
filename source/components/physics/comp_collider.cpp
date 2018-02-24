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
  
  config.is_controller = j.value("is_controller", false);
  config.is_dynamic = j.value("is_dynamic", false);
  config.is_trigger = j.value("is_trigger", false);
  config.height = j.value("height", 0.f);
  config.radius = j.value("radius", 0.f);
  config.gravity = j.value("gravity", false);

  config.group = CEngine::get().getPhysics().getFilterByName(j.value("group", "all"));
  config.mask = CEngine::get().getPhysics().getFilterByName(j.value("mask", "all"));

  if (j.count("halfExtent"))
    config.halfExtent = loadVEC3(j["halfExtent"]);

}


void TCompCollider::registerMsgs() {

  DECL_MSG(TCompCollider, TMsgEntityCreated, onCreate);
  DECL_MSG(TCompCollider, TMsgTriggerEnter, onTriggerEnter);
  DECL_MSG(TCompCollider, TMsgEntityDestroyed, onDestroy);
}

void TCompCollider::onCreate(const TMsgEntityCreated& msg) {

  CEngine::get().getPhysics().createActor(*this);
  TCompTransform *transform = get<TCompTransform>();
  lastFramePosition = transform->getPosition();
}

void TCompCollider::onDestroy(const TMsgEntityDestroyed & msg)
{
}

void TCompCollider::onTriggerEnter(const TMsgTriggerEnter& msg) {

	dbg("Entered the trigger!!!!\n");

}

void TCompCollider::update(float dt) {

	if (config.is_controller) {
		if (CHandle(this).getOwner().isValid()) {
			TCompTransform *transform = get<TCompTransform>();
			VEC3 new_pos = transform->getPosition();
			VEC3 delta_movement = new_pos - lastFramePosition;
			//velocity.x = delta_movement.x;
			//velocity.z = delta_movement.z;
			controller->move(physx::PxVec3(delta_movement.x, delta_movement.y, delta_movement.z), 0.f, dt, physx::PxControllerFilters());
			lastFramePosition = new_pos;
		}
	}

	if (config.gravity) {

		//velocity += normal_gravity;
		physx::PxControllerCollisionFlags col = controller->move(physx::PxVec3(normal_gravity.x, normal_gravity.y * dt * 10, normal_gravity.z), 0.f, dt, physx::PxControllerFilters());
		isGrounded = col.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN) ? true : false;
		//dbg("%f %f %f\n", normal_gravity.x, normal_gravity.y, normal_gravity.z);
	}

	//if (config.is_controller || config.gravity) {

	//	//controller->move(physx::PxVec3(velocity.x, velocity.y * dt, velocity.z), 0.f, dt, physx::PxControllerFilters());
	//}
}

void TCompCollider::Resize(float new_size)
{
	controller->resize((physx::PxReal)new_size);
}

void TCompCollider::SetUpVector(VEC3 new_up)
{
	controller->setUpDirection(physx::PxVec3(new_up.x, new_up.y, new_up.z));
}
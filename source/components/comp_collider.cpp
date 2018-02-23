#include "mcv_platform.h"
#include "comp_collider.h"
#include "comp_transform.h"
#include "player_controller/comp_player_controller.h"

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

  if (j.count("halfExtent"))
    config.halfExtent = loadVEC3(j["halfExtent"]);

}


void TCompCollider::registerMsgs() {

  DECL_MSG(TCompCollider, TMsgEntityCreated, onCreate);
  DECL_MSG(TCompCollider, TMsgTriggerEnter, onTriggerEnter);
}

void TCompCollider::onCreate(const TMsgEntityCreated& msg) {

  CEngine::get().getPhysics().createActor(*this);
  TCompTransform *transform = get<TCompTransform>();
  lastFramePosition = transform->getPosition();
}

void TCompCollider::onTriggerEnter(const TMsgTriggerEnter& msg) {

	dbg("Entered the trigger!!!!\n");

}

void TCompCollider::update(float dt) {

	if (config.is_controller) {

		TCompTransform *transform = get<TCompTransform>();
		VEC3 new_pos = transform->getPosition();
		VEC3 delta_movement = new_pos - lastFramePosition;
		velocity.x = delta_movement.x;
		velocity.z = delta_movement.z;
		lastFramePosition = new_pos;
	}

	if (config.gravity) {

		velocity.y -= 9.81f * dt;
	}

	if (config.is_controller || config.gravity) {

		controller->move(physx::PxVec3(velocity.x, velocity.y * dt, velocity.z), 0.f, dt, physx::PxControllerFilters());
	}

}
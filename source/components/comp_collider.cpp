#include "mcv_platform.h"
#include "comp_collider.h"
#include "comp_transform.h"
#include "comp_player_controller.h"

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
  
  config.is_character_controller = j.value("is_character_controller", false);
  config.is_dynamic = j.value("is_dynamic", false);
  config.is_trigger = j.value("is_trigger", false);
  config.height = j.value("height", 0.f);
  config.radius = j.value("radius", 0.f);
  config.gravity = j.value("gravity", false);

  if (j.count("halfExtent"))
    config.halfExtent = loadVEC3(j["halfExtent"]);
  
  lastFramePosition = VEC3::Zero;



}


void TCompCollider::registerMsgs() {

  DECL_MSG(TCompCollider, TMsgEntityCreated, onCreate);
  DECL_MSG(TCompCollider, TMsgTriggerEnter, onTriggerEnter);
}

void TCompCollider::onCreate(const TMsgEntityCreated& msg) {

  CEngine::get().getPhysics().createActor(*this);
  TCompTransform *c_my_tmx = get<TCompTransform>();
  lastFramePosition = c_my_tmx->getPosition();

}

void TCompCollider::onTriggerEnter(const TMsgTriggerEnter& msg) {

	dbg("Entered the trigger!!!!\n");

}

void TCompCollider::update(float dt) {

	if (config.is_character_controller) {

		TCompPlayerController *c_my_plyrcntlr = get<TCompPlayerController>();
		VEC3 delta = c_my_plyrcntlr->delta_movement;
		//controller->move(physx::PxVec3(delta.x, delta.y, delta.z), 0.f, dt, physx::PxControllerFilters());
		velocity.x = delta.x;
		velocity.z = delta.z;
	}

	if (config.gravity) {

		//controller->move(physx::PxVec3(0, -9.8f * dt, 0), 0.f, dt, physx::PxControllerFilters());
		velocity.y -= 9.81f * dt;
		//dbg("velocity: %f    dt: %f\n", velocity.y , &dt);
		controller->move(physx::PxVec3(velocity.x, velocity.y * dt, velocity.z), 0.f, dt, physx::PxControllerFilters());
	}

}
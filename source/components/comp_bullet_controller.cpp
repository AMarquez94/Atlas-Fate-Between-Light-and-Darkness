#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_bullet_controller.h"
#include "comp_transform.h"

DECL_OBJ_MANAGER("bullet_controller", TCompBulletController);

void TCompBulletController::debugInMenu() {
  ImGui::DragFloat("Speed", &speed, 0.1f, 0.f, 20.f);
}

void TCompBulletController::onAssignBulletOwner(const TMsgAssignBulletOwner& msg) {
  // Save who generated me
  h_sender = msg.h_owner;
  
  // Copy his position/orientation into my position/orientation
  TCompTransform *c_my_tmx = get<TCompTransform>();
  CEntity* e_sender = h_sender;
  TCompTransform *c_his_tmx = e_sender->get<TCompTransform>();
  *c_my_tmx = *c_his_tmx;
}

void TCompBulletController::registerMsgs() {
  DECL_MSG(TCompBulletController, TMsgAssignBulletOwner, onAssignBulletOwner);
}

void TCompBulletController::load(const json& j, TEntityParseContext& ctx) {
  speed = j.value("speed", 1.0f);
}

void TCompBulletController::update(float dt) {

  TCompTransform *c_tmx = get<TCompTransform>();
  assert(c_tmx);

  float amount_moved = dt * speed;
  c_tmx->setPosition(c_tmx->getPosition() + c_tmx->getFront() * amount_moved);
  
  // ---------------------
  // Check for collisions 
  auto om_tmx = getObjectManager<TCompTransform>();

  // This will store the handle close enough to me
  CHandle h_near;
  om_tmx->forEach([this, &h_near, c_tmx](TCompTransform* c) {

    // Discard myself
    if (c_tmx == c)
      return;

    float distance_to_c = VEC3::Distance( c->getPosition(), c_tmx->getPosition());
    if (distance_to_c < this->collision_radius) {
      CHandle h_candidate = CHandle(c).getOwner();
      
      // Discard my sender (the teapot)
      if(h_candidate != h_sender )
        h_near = h_candidate;
    }
  });

  // 
  if (h_near.isValid()) {
    CEntity *e_near = h_near;

    dbg("Entity %s has been hit\n", e_near->getName());

    // Notify the entity that he has been hit
    TMsgDamage msg;
    msg.h_sender = h_sender;      // Who send this bullet
    msg.h_bullet = CHandle(this); // The bullet information
    e_near->sendMsg( msg );

    // Queue my owner entity as 'destroyed'
    CHandle(this).getOwner().destroy();
  }

}


#include "mcv_platform.h"
#include "comp_rigid_anim.h"
#include "components/comp_transform.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("rigid_anim", TCompRigidAnim);

void TCompRigidAnim::load(const json& j, TEntityParseContext& ctx) {
	
	
}

void TCompRigidAnim::debugInMenu() {
  ImGui::DragFloat("Time", &current_time, 0.01f, 0.f, 10.0f);
  ImGui::DragFloat("Speed Factor", &speed_factor, 0.01f, 0.f, 5.0f);
  ImGui::Checkbox("Loops", &loops);
}
void TCompRigidAnim::registerAnimation() {
	dbg("");
}

void TCompRigidAnim::update(float dt) {
	/*
  if (!is_moving)
    return;

  // Sample the animation in the current time
  RigidAnims::TKey k;
  bool has_finished = controllers[0].sample(&k, current_time);

  // Transfer the key data to the comp transform
  TCompTransform* c_trans = get< TCompTransform >();
  c_trans->setPosition(k.pos + parent_position);
  c_trans->setRotation(k.rot);
  c_trans->setScale(VEC3(k.scale, k.scale, k.scale));

  if (has_finished) {
    if( loops )
      current_time = 0;
    // loop, change direction?, set is_moving = false...
  }

  // Advance the time
  current_time += dt * speed_factor;
  */
}

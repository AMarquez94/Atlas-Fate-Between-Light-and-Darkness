#include "mcv_platform.h"
#include "comp_rigid_anim.h"
#include "components/comp_transform.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("rigid_anim", TCompRigidAnim);

void TCompRigidAnim::load(const json& j, TEntityParseContext& ctx) {
  controller.track_name = j.value( "track", "" );
  if (controller.track_name.empty()) {
    CEntity* e = ctx.current_entity;
    assert(e);
    controller.track_name = e->getName();
  }
  assert(j.count("src") > 0);
  controller.anims = Resources.get(j["src"])->as<RigidAnims::CRigidAnimResource>();
  controller.track_index = controller.anims->findTrackIndexByName(controller.track_name);
  assert(controller.track_index != RigidAnims::CController::invalid_track_index);
  current_time = 0;
  speed_factor = j.value("speed_factor", 1.0f);
  loops = j.value("loops", true);
}

void TCompRigidAnim::debugInMenu() {
  ImGui::DragFloat("Time", &current_time, 0.01f, 0.f, 10.0f);
  ImGui::DragFloat("Speed Factor", &speed_factor, 0.01f, 0.f, 5.0f);
  ImGui::Checkbox("Loops", &loops);
}

void TCompRigidAnim::update(float dt) {

  if (!is_moving)
    return;

  // Sample the animation in the current time
  RigidAnims::TKey k;
  bool has_finished = controller.sample(&k, current_time);

  // Transfer the key data to the comp transform
  TCompTransform* c_trans = get< TCompTransform >();
  c_trans->setPosition(k.pos);
  c_trans->setRotation(k.rot);
  c_trans->setScale(k.scale);

  if (has_finished) {
    if( loops )
      current_time = 0;
    // loop, change direction?, set is_moving = false...
  }

  // Advance the time
  current_time += dt * speed_factor;
}

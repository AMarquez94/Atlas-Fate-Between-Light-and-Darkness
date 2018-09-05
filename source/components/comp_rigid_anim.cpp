#include "mcv_platform.h"
#include "comp_rigid_anim.h"
#include "components/comp_transform.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("rigid_anim", TCompRigidAnim);

void TCompRigidAnim::load(const json& j, TEntityParseContext& ctx) {

	
	
}

void TCompRigidAnim::debugInMenu() {
  ImGui::DragFloat("Time", &current_time, 0.01f, 0.f, 10.0f);
  //ImGui::DragFloat("Speed Factor", &speed_factor, 0.01f, 0.f, 5.0f);
  //ImGui::Checkbox("Loops", &loops);
}

void TCompRigidAnim::registerParentPosition(VEC3 pos) {
	parent_position = pos;
}

void TCompRigidAnim::registerAnimation(std::string animationName, std::string track_name, std::string source, float speedFactor, bool loop, int animation_id) {

	RigidAnimation aux_anim;

	aux_anim.controller.track_name = track_name;
	if (aux_anim.controller.track_name.empty()) {
		CEntity* e = CHandle(this).getOwner();
		assert(e);
		aux_anim.controller.track_name = e->getName();
	}
	aux_anim.controller.anims = Resources.get(source)->as<RigidAnims::CRigidAnimResource>();
	aux_anim.controller.track_index = aux_anim.controller.anims->findTrackIndexByName(aux_anim.controller.track_name);
	assert(controller.track_index != RigidAnims::CController::invalid_track_index);
	current_time = 0;
	aux_anim.speed_factor = speedFactor;
	aux_anim.loops = loop;
	aux_anim.animation_name = animationName;
	aux_anim.animation_id = animation_id;

	registeredAnimations.push_back(aux_anim);
}

void TCompRigidAnim::update(float dt) {
	
  if (current_animation_id == -1)
	return;

  RigidAnimation current_anim = registeredAnimations[current_animation_id];
  // Sample the animation in the current time
  RigidAnims::TKey k;
  bool has_finished = current_anim.controller.sample(&k, current_time);

  // Transfer the key data to the comp transform
  TCompTransform* c_trans = get< TCompTransform >();
  c_trans->setPosition(k.pos + parent_position);
  c_trans->setRotation(k.rot);

  //dbg("%f   %f   %f  \n", k.pos.x, k.pos.y, k.pos.z);
  c_trans->setScale(VEC3(k.scale, k.scale, k.scale));

  dbg("%f\n", current_time);

  if (has_finished) {
    if(current_anim.loops )
      current_time = 0.0f;
    // loop, change direction?, set is_moving = false...
  }

  // Advance the time
  current_time += dt * current_anim.speed_factor;
  
}

bool TCompRigidAnim::playAnimation(int anim_id) {

	if (registeredAnimations.size() - 1 < anim_id)
		return false;
	current_time = 0.0f;
	current_animation_id = anim_id;
	return true;
}

void TCompRigidAnim::cancelAnimation() {
	current_animation_id = -1;
}

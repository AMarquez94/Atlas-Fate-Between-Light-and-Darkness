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

void TCompRigidAnim::registerParentTransform(VEC3 pos, QUAT rot) {
	CEntity * en = CHandle(this).getOwner();
	TCompTransform *comp_trans = en->get<TCompTransform>();
	
	parent_position = pos;
	parent_rotation = rot;
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
	assert(aux_anim.controller.track_index != RigidAnims::CController::invalid_track_index);
	if (aux_anim.controller.isEmpty()) {
		withoutKeys.push_back(true);
	} else {
		withoutKeys.push_back(false);
	}
	current_time = 0;
	aux_anim.speed_factor = speedFactor;
	aux_anim.loops = loop;
	aux_anim.animation_name = animationName;
	aux_anim.animation_id = animation_id;

	registeredAnimations.push_back(aux_anim);
}

void TCompRigidAnim::update(float dt) {
	
  if (current_animation_id == -1 || withoutKeys[current_animation_id])
	return;

  RigidAnimation current_anim = registeredAnimations[current_animation_id];
  // Sample the animation in the current time
  RigidAnims::TKey k;
  bool has_finished = current_anim.controller.sample(&k, current_time);
  
  if (current_time == 0.0f) {
	  last_k_pos = k.pos;
  }

  TCompTransform* c_trans = get< TCompTransform >();

  VEC3 delta_k_movement = k.pos - last_k_pos;
  //SETTING-POSITIOM
  c_trans->setPosition( c_trans->getPosition() + c_trans->getLeft() * delta_k_movement.x);
  c_trans->setPosition( c_trans->getPosition() + c_trans->getUp() * delta_k_movement.y);
  c_trans->setPosition( c_trans->getPosition() + c_trans->getFront() * delta_k_movement.z);

  //SETTING-ROTATION
  c_trans->setRotation( k.rot * parent_rotation );

  //SETTING-SCALE
  c_trans->setScale(VEC3(k.scale, k.scale, k.scale));

  if (has_finished) {
	if (next_animation_id != -1) {
		current_animation_id = next_animation_id;
		next_animation_id = -1;
		current_time = 0.0f;
	}
    if(current_anim.loops )
      current_time = 0.0f;
    // loop, change direction?, set is_moving = false...
  }
  last_k_pos = k.pos;
  // Advance the time
  current_time += dt * current_anim.speed_factor;
  
}

bool TCompRigidAnim::playAnimation(int anim_id) {

	if (registeredAnimations.size() - 1 < anim_id)
		return false;
	if (current_animation_id == -1) {
		current_animation_id = anim_id;
		current_time = 0.0f;
	}
	else 
	{ 
		next_animation_id = anim_id; 
	}
	return true;
}

void TCompRigidAnim::cancelAnimation() {
	current_animation_id = -1;
}

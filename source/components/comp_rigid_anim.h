#ifndef INC_COMP_RIGID_ANIM_H_
#define INC_COMP_RIGID_ANIM_H_

#include "geometry/transform.h"
#include "geometry/rigid_anim.h"

class CRigidAnimResource;

struct TCompRigidAnim : public TCompBase {

  struct RigidAnimation {

	std::string animation_name;
	int animation_id;
	RigidAnims::CController controller;
  	float speed_factor = 1.0f;
	bool loops = false;

  };

  std::vector<RigidAnimation> registeredAnimations;

  int					  next_animation_id = -1;
  int					  current_animation_id = -1;
  float                   current_time = 0.f;
  bool withoutKeys = false;

  VEC3					  parent_position;
  QUAT					  parent_rotation;

  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void debugInMenu();

  bool playAnimation(int anim_id);
  void cancelAnimation();

  void registerParentTransform(VEC3 pos, QUAT rot);

  void registerAnimation(std::string animationName, std::string track_name, std::string source, float speedFactor, bool loop, int animation_id);
  DECL_SIBLING_ACCESS();
};

#endif

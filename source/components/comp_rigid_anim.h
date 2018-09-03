#ifndef INC_COMP_RIGID_ANIM_H_
#define INC_COMP_RIGID_ANIM_H_

#include "geometry/transform.h"
#include "geometry/rigid_anim.h"

class CRigidAnimResource;

struct TCompRigidAnim : public TCompBase {
  std::vector<RigidAnims::CController> controllers;
  float                   current_time = 0.f;
  bool                    loops = true;
  bool                    is_moving = true;
  float                   speed_factor = 1.0f;
  VEC3					  parent_position;


  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void debugInMenu();
  DECL_SIBLING_ACCESS();
};

#endif

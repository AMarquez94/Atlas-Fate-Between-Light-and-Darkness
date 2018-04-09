#include "mcv_platform.h"
#include "comp_bone_tracker.h"
#include "components/comp_transform.h"
#include "comp_skeleton.h"
#include "cal3d/cal3d.h"
#include "cal3d2engine.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("bone_tracker", TCompBoneTracker);

void TCompBoneTracker::load(const json& j, TEntityParseContext& ctx) {
  bone_name   = j.value("bone", "");
  parent_name = j.value("parent", "");
  assert(!bone_name.empty());
  assert(!parent_name.empty());

  CEntity* e_parent = ctx.findEntityByName(parent_name);
  if (e_parent) 
    h_skeleton = e_parent->get<TCompSkeleton>();
}

void TCompBoneTracker::update(float dt) {
  TCompSkeleton* c_skel = h_skeleton;

  if (c_skel == nullptr) {
    // Search the parent entity by name
    CEntity* e_entity = getEntityByName(parent_name);
    if (!e_entity)
      return;
    // The entity I'm tracking should have an skeleton component.
    h_skeleton = e_entity->get<TCompSkeleton>();
    assert(h_skeleton.isValid());
    c_skel = h_skeleton;
  }

  if (bone_id == -1) {
    bone_id = c_skel->model->getCoreModel()->getCoreSkeleton()->getCoreBoneId(bone_name);
    // The skeleton don't have the bone with name 'bone_name'
    assert(bone_id != -1);
  }
  
  // Access to the bone 'bone_id' of the skeleton
  auto cal_bone = c_skel->model->getSkeleton()->getBone(bone_id);
  QUAT rot = Cal2DX(cal_bone->getRotationAbsolute());
  VEC3 pos = Cal2DX(cal_bone->getTranslationAbsolute());

  // Apply the cal3d pos&rot to my entity owner
  TCompTransform* tmx = get<TCompTransform>();
  tmx->setPosition(pos);
  tmx->setRotation(rot);
}

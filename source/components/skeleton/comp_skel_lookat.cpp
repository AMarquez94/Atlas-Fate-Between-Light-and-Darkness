#include "mcv_platform.h"
#include "comp_skel_lookat.h"
#include "components/comp_transform.h"
#include "components/skeleton/comp_skeleton.h"
#include "cal3d/cal3d.h"
#include "cal3d2engine.h"
#include "entity/entity_parser.h"
#include "game_core_skeleton.h"

DECL_OBJ_MANAGER("skel_lookat", TCompSkelLookAt);

void TCompSkelLookAt::load(const json& j, TEntityParseContext& ctx) {
  if( j.count("target"))
    target = loadVEC3( j["target"] );
  amount = j.value("amount", amount);
  target_transition_factor = j.value("target_transition_factor", target_transition_factor);

  target_entity_name = j.value("target_entity_name", "");
}

void TCompSkelLookAt::update(float dt) {
  TCompSkeleton* c_skel = h_skeleton;

  if (!target_entity_name.empty() && !h_target_entity.isValid()) {
    h_target_entity = getEntityByName(target_entity_name);
  }

  // If we have a target entity, use it to assign a target position
  CEntity* e = h_target_entity;
  if (e) {
    TCompTransform* t = e->get<TCompTransform>();
    // We should ensure the target position is not moving abruptly
    // to avoid large changes in the skeleton reorientation
    // make the target smoothly change between positions
    VEC3 new_target = t->getPosition();
    target = target * target_transition_factor + new_target * (1.0f - target_transition_factor);
  }

  if (c_skel == nullptr) {
    // Search the parent entity by name
    CEntity* e_entity = CHandle(this).getOwner();
    if (!e_entity)
      return;
    // The entity I'm tracking should have an skeleton component.
    h_skeleton = e_entity->get<TCompSkeleton>();
    assert(h_skeleton.isValid());
    c_skel = h_skeleton;
  }

  // The cal3d skeleton instance
  CalSkeleton* skel = c_skel->model->getSkeleton();

  // The set of bones to correct
  auto core = (CGameCoreSkeleton*)c_skel->model->getCoreModel();
  for (auto& it : core->lookat_corrections)
    it.apply(skel, target, amount);
}

void TCompSkelLookAt::debugInMenu() {
  ImGui::InputFloat3("Target", &target.x);
  ImGui::LabelText( "Target Name", "%s", target_entity_name.c_str() );
  ImGui::DragFloat("Amount", &amount, 0.01f, 0.f, 1.0f);
  ImGui::DragFloat("Transition Factor", &target_transition_factor, 0.01f, 0.f, 1.0f);
}

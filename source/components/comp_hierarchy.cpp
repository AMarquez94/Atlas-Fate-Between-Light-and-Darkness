#include "mcv_platform.h"
#include "comp_hierarchy.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("hierarchy", TCompHierarchy);

void TCompHierarchy::load(const json& j, TEntityParseContext& ctx) {
  assert(j.count("parent") );
  parent_name = j.value("parent", "");
  // Relative transform is loaded as any other json transform
  CTransform::load(j);
}

void TCompHierarchy::registerMsgs() {
  DECL_MSG(TCompHierarchy, TMsgHierarchyGroupCreated, onGroupCreated);
}

void TCompHierarchy::convertTotalTransformToRelative(VEC3 objPosition, QUAT objRotation, float objScale, VEC3 & resultPosition, QUAT & resultRotation, float & resultScale)
{
    TCompTransform* c_parent_transform = h_parent_transform;
    TCompTransform* my_transform = h_my_transform;

    MAT44 myMatrix = c_parent_transform->asMatrix();

    resultScale = objScale / c_parent_transform->getScale();

    VEC3 delta_pos_rotated = (objPosition - c_parent_transform->getPosition()) / c_parent_transform->getScale();
    QUAT parentRotation = c_parent_transform->getRotation();
    QUAT parentRotationInverse;
    parentRotation.Inverse(parentRotationInverse);
    resultPosition = VEC3::Transform(delta_pos_rotated, parentRotationInverse);

    resultRotation = objRotation / c_parent_transform->getRotation();
}

QUAT TCompHierarchy::getRelativeLookAt(VEC3 new_target)
{
    TCompTransform* lanternPos = get<TCompTransform>();
    VEC3 objectivePos = lanternPos->getPosition();
    CEntity* eParent = h_parent;
    TCompTransform* parentPos = eParent->get<TCompTransform>();
    float yaw, pitch, roll;
    parentPos->getYawPitchRoll(&yaw, &pitch, &roll);
    QUAT objectiveRotation = lanternPos->getLookAt(objectivePos, new_target, roll);

    VEC3 resultPos;
    QUAT resultRot;
    float resultScale;

    convertTotalTransformToRelative(objectivePos, objectiveRotation, 1.f, resultPos, resultRot, resultScale);
    return resultRot;
}

void TCompHierarchy::onGroupCreated(const TMsgHierarchyGroupCreated& msg) {
  // I prefer to wait until the group is loaded to resolve my parent
  setParentEntity(msg.ctx.findEntityByName(parent_name));
}

void TCompHierarchy::debugInMenu() {
  ImGui::LabelText("Parent Name", "%s", parent_name.c_str());
  CHandle h_parent_entity = h_parent_transform.getOwner();
  if (h_parent_entity.isValid())
    h_parent_entity.debugInMenu();
  CTransform::debugInMenu();
} 

void TCompHierarchy::setParentEntity(CHandle new_h_parent) {
  CEntity* e_parent = new_h_parent;
  if (e_parent) {
    // Cache the two handles: the comp_transform of the entity I'm tracing, and my comp_transform
    h_parent_transform = e_parent->get<TCompTransform>();
    CEntity* e_my_owner = CHandle(this).getOwner();
    h_my_transform = e_my_owner->get<TCompTransform>();
    parent_name = e_parent->getName();
    h_parent = new_h_parent;
  }
  else {
    // Invalidate previous contents
    h_parent_transform = CHandle();
    h_my_transform = CHandle();
    h_parent = CHandle();
  }
}

void TCompHierarchy::update(float dt) {

  // My parent world transform
  TCompTransform* c_parent_transform = h_parent_transform;
  if (!c_parent_transform)
    return;

  // My Sibling comp transform
  TCompTransform* c_my_transform = h_my_transform;
  assert(c_my_transform);

  // Combine the current world transform with my 
  c_my_transform->set(c_parent_transform->combineWith(*this) );
}
#ifndef INC_COMP_HIERARCHY_TRANSFORM_H_
#define INC_COMP_HIERARCHY_TRANSFORM_H_

#include "geometry/transform.h"

struct TMsgEntitiesGroupCreated;

// CTransform is the LOCAL transform with respect to my parent!!
// This component has a run-time penalty as everyframe we need to recompute
// our transform based on my parent transform.
// Do not use it for static meshes
struct TCompHierarchy : public CTransform, public TCompBase {
  CHandle     h_parent_transform;
  CHandle     h_my_transform;
  std::string parent_name;          // entity name as search in the ctx
  CHandle     h_parent;

  void setParentEntity(CHandle new_h_parent);

  void onGroupCreated(const TMsgHierarchyGroupCreated& msg);
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void debugInMenu();
  static void registerMsgs();
  void convertTotalTransformToRelative(VEC3 objPosition, QUAT objRotation, float objScale, VEC3& resultPosition, QUAT& resultRotation, float& resultScale);
  QUAT getRelativeLookAt(VEC3 new_target);
  DECL_SIBLING_ACCESS();
};


#endif

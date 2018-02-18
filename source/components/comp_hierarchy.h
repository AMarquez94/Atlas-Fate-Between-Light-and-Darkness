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

  void setParentEntity(CHandle new_h_parent);

  void onGroupCreated(const TMsgEntitiesGroupCreated& msg);
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void debugInMenu();
  static void registerMsgs();
  DECL_SIBLING_ACCESS();
};


#endif

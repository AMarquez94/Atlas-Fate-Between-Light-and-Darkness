#ifndef INC_COMPONENT_AABB_H_
#define INC_COMPONENT_AABB_H_

#include "geometry/geometry.h"
#include "components/comp_base.h"

struct TMsgEntityCreated;

// Component base to derivce the AbsAABB & LocalAABB
struct TCompAABB : public AABB, public TCompBase {
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void renderDebug();
  void updateFromSiblingsLocalAABBs(CHandle h_entity);
};

// This is required to use the culling system
struct TCompAbsAABB : public TCompAABB {
  void onCreate(const TMsgEntityCreated&);
  void onGroupCreated(const TMsgEntitiesGroupCreated& msg);
  static void registerMsgs();
  DECL_SIBLING_ACCESS();
};

// Updates AbsAABB from LocalAABB and CompTransform
// Use this component only if the object is moving in realtime
// during gameplay, otherwise, we are loosing time.
struct TCompLocalAABB : public TCompAABB {
  void update(float dt);
  void renderDebug();
  void onCreate(const TMsgEntityCreated&);
  void onGroupCreated(const TMsgEntitiesGroupCreated& msg);
  static void registerMsgs();
  DECL_SIBLING_ACCESS();
};

#endif

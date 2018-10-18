#ifndef INC_COMP_TIED_HIERARCHY_H_
#define INC_COMP_TIED_HIERARCHY_H_

#include "geometry/transform.h"

struct TMsgEntitiesGroupCreated;

struct TCompTiedHierarchy : public CTransform, public TCompBase {

    CHandle     h_parent_transform;
    CHandle     h_my_transform;
    std::string parent_name;          // entity name as search in the ctx
    CHandle     h_parent;
    VEC3        orig_pos;
    VEC3        orig_scale = VEC3::One;
    VEC3        orig_rot = VEC3::Zero;

    void setParentEntity(CHandle new_h_parent);
    void onGroupCreated(const TMsgHierarchyGroupCreated& msg);

    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    void debugInMenu();
    static void registerMsgs();

    DECL_SIBLING_ACCESS();
};


#endif

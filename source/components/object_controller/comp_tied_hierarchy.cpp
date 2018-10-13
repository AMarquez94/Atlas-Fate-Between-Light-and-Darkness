#include "mcv_platform.h"
#include "comp_tied_hierarchy.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("tied_hierarchy", TCompTiedHierarchy);

void TCompTiedHierarchy::load(const json& j, TEntityParseContext& ctx) {

    assert(j.count("parent"));
    parent_name = j.value("parent", "");
    orig_pos = loadVEC3(j["pos"]);
    if (j.count("scale"))
    {
        orig_scale = loadVEC3(j["scale"]);
    }
    // Relative transform is loaded as any other json transform
    CTransform::load(j);
}

void TCompTiedHierarchy::registerMsgs() {

    DECL_MSG(TCompTiedHierarchy, TMsgHierarchyGroupCreated, onGroupCreated);
}

void TCompTiedHierarchy::onGroupCreated(const TMsgHierarchyGroupCreated& msg) {

    setParentEntity(msg.ctx.findEntityByName(parent_name));
}

void TCompTiedHierarchy::debugInMenu() {

    ImGui::LabelText("Parent Name", "%s", parent_name.c_str());
    CHandle h_parent_entity = h_parent_transform.getOwner();
    if (h_parent_entity.isValid())
        h_parent_entity.debugInMenu();
    CTransform::debugInMenu();
}

void TCompTiedHierarchy::setParentEntity(CHandle new_h_parent) {

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

void TCompTiedHierarchy::update(float dt) {

    // My parent world transform
    TCompTransform* c_parent_transform = h_parent_transform;
    if (!c_parent_transform)
        return;

    // Dirty component, no time to make this clean.
    TCompTransform* c_my_transform = h_my_transform;
    CTransform new_t;
    new_t.setPosition(orig_pos + c_parent_transform->getPosition());
    new_t.setScale(orig_scale);

    c_my_transform->set(new_t);
}
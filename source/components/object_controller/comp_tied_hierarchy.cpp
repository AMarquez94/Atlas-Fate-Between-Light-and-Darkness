#include "mcv_platform.h"
#include "comp_tied_hierarchy.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("tied_hierarchy", TCompTiedHierarchy);

void TCompTiedHierarchy::load(const json& j, TEntityParseContext& ctx) {

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

    CEntity* e_my_owner = CHandle(this).getOwner();
    h_my_transform = e_my_owner->get<TCompTransform>();
}

void TCompTiedHierarchy::debugInMenu() {

    ImGui::DragFloat3("Pos", &orig_pos.x);
    ImGui::DragFloat3("Rot", &orig_rot.x);
    ImGui::DragFloat3("Scale", &orig_scale.x);

    CHandle h_parent_entity = h_parent_transform.getOwner();
    if (h_parent_entity.isValid())
        h_parent_entity.debugInMenu();
    CTransform::debugInMenu();
}

void TCompTiedHierarchy::update(float dt) {

    // My parent world transform
    //TCompTransform* c_parent_transform = h_parent_transform;
    //if (!c_parent_transform)
    //    return;

    // Dirty component, no time to make this clean.
    TCompTransform* c_my_transform = h_my_transform;

    CTransform new_t;
    CHandle ent = EngineCameras.getCurrentCamera();

    if (ent.isValid()) {
        CEntity * n_ent = ent;
        TCompTransform * pos = n_ent->get<TCompTransform>();
        if (pos) {
            new_t.setPosition(orig_pos + pos->getPosition());
            new_t.setScale(orig_scale);
            new_t.setRotation(QUAT::CreateFromYawPitchRoll(orig_rot.x, orig_rot.y, orig_rot.z));
            c_my_transform->set(new_t);
        }
    }
}
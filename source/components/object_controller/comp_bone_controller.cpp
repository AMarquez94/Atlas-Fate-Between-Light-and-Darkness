#include "mcv_platform.h"
#include "comp_bone_controller.h"
#include "components/comp_transform.h"
#include "components/comp_hierarchy.h"
#include "components/physics/comp_rigidbody.h"
#include "components/comp_group.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("bone_controller", TCompBoneController);

void TCompBoneController::debugInMenu() {


}

void TCompBoneController::load(const json& j, TEntityParseContext& ctx) {

    _bone = j.value("bone", "root");
    _target = j.value("target", "root");
    assert(!_bone.empty());
    assert(!_target.empty());
}

void TCompBoneController::registerMsgs() {

    DECL_MSG(TCompBoneController, TMsgSceneCreated, onSceneCreated);
}

void TCompBoneController::onSceneCreated(const TMsgSceneCreated& msg) {

    h_target = getEntityByName(_target);

    CEntity * ent = h_target;
    target_skeleton = ent->get<TCompSkeleton>();
}

void TCompBoneController::update(float dt) {

    // Dirty and nasty way of doing this.
    VEC3 pos = target_skeleton->getBonePosition(_bone);
    TCompTransform * my_trans = get<TCompTransform>();

    // In case it has a hierarchy component...
    CEntity * ent = h_target;
    TCompHierarchy * t_hier = ent->get<TCompHierarchy>();

    if (t_hier) {
        MAT44 parent_pos = my_trans->asMatrix().Invert();
        pos = VEC3::Transform(pos, parent_pos);
    }

    t_hier->setPosition(pos);

}
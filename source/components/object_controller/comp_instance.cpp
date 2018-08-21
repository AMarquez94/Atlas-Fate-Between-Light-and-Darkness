#include "mcv_platform.h"
#include "comp_instance.h"
#include "components/comp_transform.h"
#include "components/comp_hierarchy.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("instance", TCompInstance);

void TCompInstance::load(const json& j, TEntityParseContext& ctx) {

    _instance_mesh = j.value("mesh", "data/meshes/GeoSphere001.instanced_mesh");
    EngineInstancing.parseInstance(j, ctx);
}

void TCompInstance::registerMsgs() {

    DECL_MSG(TCompInstance, TMsgEntityCreated, onMsgEntityCreated);
    DECL_MSG(TCompInstance, TMsgEntitiesGroupCreated, onGroupCreated);
}

void TCompInstance::onGroupCreated(const TMsgEntitiesGroupCreated& msg) {

    TCompTransform * self_transform = get<TCompTransform>();
    MAT44 new_rot = MAT44::CreateFromQuaternion(self_transform->getRotation());
    MAT44 new_trans = MAT44::CreateTranslation(self_transform->asMatrix().Translation());
    MAT44 posz = MAT44::CreateScale(VEC3(4, 1, 2)) * new_rot * new_trans;
    _index = EngineInstancing.addInstance(_instance_mesh, posz);
}

void TCompInstance::onMsgEntityCreated(const TMsgEntityCreated& msg) {


    //TCompTransform * self_transform = get<TCompTransform>();
    //MAT44 w_matrix = self_transform->asMatrix();
    //_index = EngineInstancing.addInstance(_instance_mesh, w_matrix);
}

/* Update the world matrix of the given instance */
void TCompInstance::update(float dt) {

    TCompTransform * self_transform = get<TCompTransform>();
    //_instance->world = self_transform->asMatrix();
    
    MAT44 new_rot = MAT44::CreateFromQuaternion(self_transform->getRotation());
    MAT44 new_trans = MAT44::CreateTranslation(self_transform->asMatrix().Translation());
    MAT44 posz = MAT44::CreateScale(VEC3(4,1,2)) * new_rot * new_trans;
    EngineInstancing.updateInstance(_instance_mesh, _index, posz);
}
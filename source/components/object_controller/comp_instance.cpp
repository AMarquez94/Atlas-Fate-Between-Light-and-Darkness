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
}

void TCompInstance::registerMsgs() {

    DECL_MSG(TCompInstance, TMsgEntityCreated, onMsgEntityCreated);
}

void TCompInstance::onMsgEntityCreated(const TMsgEntityCreated& msg) {

    TCompTransform * self_transform = get<TCompTransform>();
    MAT44 w_matrix = self_transform->asMatrix();
    _index = EngineInstancing.addInstance(_instance_mesh, w_matrix);
}

/* Update the world matrix of the given instance */
void TCompInstance::update(float dt) {

    TCompTransform * self_transform = get<TCompTransform>();
    //_instance->world = self_transform->asMatrix();
    EngineInstancing.updateInstance(_instance_mesh, _index, self_transform->asMatrix());
}
#include "mcv_platform.h"
#include "comp_instance.h"
#include "components/comp_transform.h"
#include "components/comp_hierarchy.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("instance", TCompInstance);

void TCompInstance::load(const json& j, TEntityParseContext& ctx) {

    _type = j.value("type", "default");
    _instance_mesh = j.value("mesh", "data/meshes/GeoSphere001.instanced_mesh");
    EngineInstancing.parseInstance(j, ctx);
}

void TCompInstance::registerMsgs() {

    DECL_MSG(TCompInstance, TMsgEntityCreated, onMsgEntityCreated);
    DECL_MSG(TCompInstance, TMsgEntitiesGroupCreated, onGroupCreated);
}

void TCompInstance::onGroupCreated(const TMsgEntitiesGroupCreated& msg) {

    TCompTransform * self_transform = get<TCompTransform>();
    const CTransform r_transform = msg.ctx.root_transform;
   
    //dbg(" %s %f %f %f \n", _instance_mesh, pos.x, pos.y, pos.z);
    _index = EngineInstancing.addInstance(_instance_mesh, _type, self_transform->asMatrix());
}

void TCompInstance::onMsgEntityCreated(const TMsgEntityCreated& msg) {

}

/* Update the world matrix of the given instance */
void TCompInstance::update(float dt) {

    TCompTransform * self_transform = get<TCompTransform>();
    EngineInstancing.updateInstance(_instance_mesh, _index, self_transform->asMatrix());
}
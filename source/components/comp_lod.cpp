#include "mcv_platform.h"
#include "comp_lod.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "components/comp_render.h"
#include "components/comp_aabb.h"
#include "components/comp_transform.h"
#include "components/comp_camera.h"
#include "render/render_manager.h"

DECL_OBJ_MANAGER("lod", TCompLOD);

// TODO. If the children are prefabs and contains other entities
// we currently will not notify those entities as the msg is only
// send to the entity names associated to me in the json
// One option is to make the comp_group component forward the 
// TMsgSetVisible msg to his children.

// TODO camera name should be removed and not computed everytime

void TCompLOD::debugInMenu() {
    ImGui::DragFloat("Threshold", &lod_threshold, 0.001f, 0.0f, 0.1f);
    ImGui::Text("Current: %f", lod_level);
    ImGui::Text("Using LOD: %s", using_lod ? "YES" : "NO");
    ImGui::Text("Distance To Camera: %f", distance_to_camera);
    ImGui::Text("My Size: %f", my_size);

    ImGui::InputFloat3("AABB.center", &aabb.Center.x);
    ImGui::InputFloat3("AABB.half", &aabb.Extents.x);

    if (ImGui::TreeNode("Children")) {
        for (auto h : handles)
            h.debugInMenu();
        ImGui::TreePop();
    }
}

void TCompLOD::load(const json& j, TEntityParseContext& ctx) {
    names = j.value< std::vector<std::string> >("children", {});
    for (auto& n : names) {
        CHandle h = ctx.findEntityByName(n);
        assert(h.isValid());
        handles.push_back(h);
    }
    // In case the children AABB are moving each frame
    children_dynamic = j.value("children_dynamic", false);
    lod_threshold = j.value("threshold", lod_threshold);
    h_camera = EngineCameras.getCurrentCamera();
}

void TCompLOD::updateAABBFromChildren() {

    int nchildren = 0;

    for (auto h : handles) {
        CEntity* e = h;
        TCompAbsAABB* c_aabb = e->get<TCompAbsAABB>();
        if (c_aabb) {
            AABB* child_aabb = c_aabb;
            if (nchildren == 0)
                aabb = *child_aabb;
            else
                AABB::CreateMerged(aabb, aabb, *child_aabb);
            ++nchildren;
        }
    }

    aabb_update_required = false;
}

// 
void TCompLOD::updateLevel() {

    // Compute a ratio between my size and the distance to the camera
    VEC3 my_half_size = aabb.Extents;
    my_size = my_half_size.Length();

    TCompTransform* my_trans = get<TCompTransform>();
    assert(my_trans);

    //Engine.getRender().getName.getCameras().setOutputCamera
    CEntity* e_camera = h_camera;
    if (!e_camera)
        return;
    assert(e_camera);
    TCompCamera* camera_render = e_camera->get<TCompCamera>();
    assert(camera_render);

    VEC3 camera_pos = camera_render->getPosition();
    VEC3 my_pos = my_trans->getPosition();
    distance_to_camera = (camera_pos - my_pos).Length();

    lod_level = my_size / distance_to_camera;
    using_lod = lod_level < lod_threshold;
}

void TCompLOD::applyLODStatus() {

    // this.visible = ( using_lod );
    TMsgSetVisible msg = { using_lod };
    CHandle(this).getOwner().sendMsg(msg);

    // children.visible = ( !using_lod );
    msg.visible = !msg.visible;
    for (auto h : handles)
        h.sendMsg(msg);
}

void TCompLOD::update(float delta) {

    if (children_dynamic || aabb_update_required)
        updateAABBFromChildren();

    // TODO remove this line, cache the camera_pos
    // somewhere
    h_camera = ::getEntityByName(camera_name);

    bool was_using_lod = using_lod;
    updateLevel();

    if (was_using_lod != using_lod)
        applyLODStatus();

}


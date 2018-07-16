#include "mcv_platform.h"
#include "comp_render.h"
#include "comp_transform.h"
#include "skeleton/comp_skeleton.h"

DECL_OBJ_MANAGER("render", TCompRender);

#include "render/render_objects.h"
#include "render/texture/texture.h"
#include "render/texture/material.h"
#include "render/render_utils.h"
#include "render/render_manager.h"
#include "entity/entity_parser.h"


TCompRender::~TCompRender() {
    // Delete all references of me in the render manager
    CRenderManager::get().delRenderKeys(CHandle(this));
}

// --------------------------------------------
void TCompRender::onDefineLocalAABB(const TMsgDefineLocalAABB& msg) {
    AABB::CreateMerged(*msg.aabb, *msg.aabb, aabb);
}

// --------------------------------------------
void TCompRender::onSetVisible(const TMsgSetVisible& msg) {

    // If the flag has not changed, nothing to change
    if (global_enabled == msg.visible)
        return;

    // Now keep the new value
    global_enabled = msg.visible;

    // This means we were visible, so we only need to remove my self
    if (!global_enabled) {
        CRenderManager::get().delRenderKeys(CHandle(this));
    }
    else {
        // We come from being invisible, so just add my render keys
        refreshMeshesInRenderManager(false);
    }
}

void TCompRender::registerMsgs() {
    DECL_MSG(TCompRender, TMsgDefineLocalAABB, onDefineLocalAABB);
    DECL_MSG(TCompRender, TMsgSetVisible, onSetVisible);
}

void TCompRender::debugInMenu() {

    ImGui::ColorEdit4("Color", &color.x);
    ImGui::ColorEdit4("Self Color", &self_color.x);
    ImGui::DragFloat("Self Intensity", &self_intensity, 0.01f, 0.f, 50.f);

    bool changed = false;
    for (auto& mwm : meshes) {
        ImGui::PushID(&mwm);
        // if the users changed the 'enabled' flag, save it
        changed |= ImGui::Checkbox("Enabled", &mwm.enabled);
        ImGui::LabelText("Mesh", "%s", mwm.mesh->getName().c_str());
        for (auto &m : mwm.materials) {
            if (m)
                ((CMaterial*)m)->debugInMenu();
        }
        ImGui::PopID();
    }

    // Notify the rendermanager that we should regenerate our contents
    if (changed) {
        TMsgSetVisible msg = { false };
        CHandle(this).getOwner().sendMsg(msg);
    }
        //refreshMeshesInRenderManager(true);
}

void TCompRender::renderMeshes() {
  TCompTransform* transform = get<TCompTransform>();
  assert(transform);

  // Ifwe have an skeleton, make sure the required bones are active and updated
  TCompSkeleton* skel = get<TCompSkeleton>();
  if (skel) {
    skel->updateCtesBones();
    skel->cb_bones.activate();
  }

  for (auto& mwm : meshes) {
    if (!mwm.enabled || skel)
      continue;

    renderMesh(mwm.mesh, transform->asMatrix(), color);
  }
}

void TCompRender::renderDebug() {

  activateRSConfig(RSCFG_WIREFRAME);
  renderMeshes();
  activateRSConfig(RSCFG_DEFAULT);
}

void TCompRender::loadMesh(const json& j, TEntityParseContext& ctx) {

    CHandle(this).setOwner(ctx.current_entity);
    CMeshWithMaterials mwm;
    std::string name_mesh = j.value("mesh", "axis.mesh");
    mwm.mesh = Resources.get(name_mesh)->as<CRenderMesh>();

    if (j.count("materials")) {
        auto& j_mats = j["materials"];
        assert(j_mats.is_array());
        for (size_t i = 0; i < j_mats.size(); ++i) {
            // Allow to define a null and not render that material idx of the mesh
            const CMaterial* material = nullptr;
            if (j_mats[i].is_string()) {
                std::string name_material = j_mats[i];
                material = Resources.get(name_material)->as<CMaterial>();
            }
            mwm.materials.push_back(material);
        }
        assert(mwm.materials.size() <= mwm.mesh->getSubGroups().size());
    }
    else {
        const CMaterial* material = Resources.get("data/materials/solid.material")->as<CMaterial>();
        mwm.materials.push_back(material);
    }

    mwm.enabled = j.value("enabled", true);

    // If there is a color in the json, read it
    if (j.count("color"))
        color = loadVEC4(j["color"]);

    //self_intensity = j.value("self_intensity", 1.0f);
    AABB::CreateMerged(aabb, aabb, mwm.mesh->getAABB());

    meshes.push_back(mwm);
}

void TCompRender::load(const json& j, TEntityParseContext& ctx) {

    // Reset the AABB
    aabb.Center = VEC3(0, 0, 0);
    aabb.Extents = VEC3(0, 0, 0);

    // We expect an array of things to render: mesh + materials, mesh + materials, ..
    if (j.is_array()) {
        for (size_t i = 0; i < j.size(); ++i)
            loadMesh(j[i], ctx);
    }
    else {
        // We accept not receiving an array of mesh inside the comp_render, for handle files
        loadMesh(j, ctx);
    }

    refreshMeshesInRenderManager(true);

    if (j.count("COLOR"))
        color = loadVEC4(j["color"]);

    self_color = j.count("self_color") ? loadVEC4(j["self_color"]) : VEC4(1, 1, 1, 1);
}

void TCompRender::refreshMeshesInRenderManager(bool delete_me_from_keys) {
    CHandle h_me = CHandle(this);
    CRenderManager::get().delRenderKeys(h_me);

    // The house and the trees..
    for (auto& mwm : meshes) {

        // Do not register disabled meshes
        if (!mwm.enabled || !global_enabled)
            continue;

        // All materials of the house...
        uint32_t idx = 0;
        for (auto& m : mwm.materials) {
            // Supporting null materials to discard submeshes
            if (m) {
                CRenderManager::get().addRenderKey(
                    h_me,
                    mwm.mesh,
                    m,
                    idx
                );
            }
            ++idx;
        }
    }
}

// Only allow us to change one material per mesh, multimaterial won't work.
// In case of multimaterial needed, load from file.
void TCompRender::setMaterial(const std::string &name) {

    CMaterial * material = (CMaterial*)Resources.get(name)->as<CMaterial>();

    // The house and the trees..
    for (auto& mwm : meshes) {

        // Do not register disabled meshes
        if (!mwm.enabled || !global_enabled)
            continue;

        mwm.materials.clear();
        mwm.materials.push_back(material);
    }

    refreshMeshesInRenderManager(true);
}

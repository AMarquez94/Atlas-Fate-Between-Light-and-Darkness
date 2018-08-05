#include "mcv_platform.h"
#include "render_manager.h"
#include "render/render.h"
#include "render/texture/material.h"
#include "components/comp_transform.h"
#include "components/comp_render.h"
#include "render_objects.h"
#include "components/comp_render.h"
#include "components/comp_culling.h"
#include "components/comp_aabb.h"
#include "components/skeleton/comp_skeleton.h"
#include "shaders/vertex_shader.h"

static CRenderManager the_render_manager;
CRenderManager& CRenderManager::get() {
    return the_render_manager;
}

// To be able to compare the RenderKeys with a category_id
bool operator<(CRenderManager::TRenderKey& k1, uint32_t category_id) {

    return k1.material->tech->getCategoryID() < category_id;
}

bool operator<(uint32_t category_id, CRenderManager::TRenderKey& k1) {

    return category_id < k1.material->tech->getCategoryID();
}

// General function to render keys
bool CRenderManager::sortRenderKeys(const TRenderKey& k1, const TRenderKey& k2) {

    // Category
    if (k1.material->tech->getCategoryID() != k2.material->tech->getCategoryID())
        return k1.material->tech->getCategoryID() < k2.material->tech->getCategoryID();
    // NonSkin -> Skin  
    if (k1.material->tech->usesSkin() != k2.material->tech->usesSkin())
        return k1.material->tech->usesSkin() < k2.material->tech->usesSkin();
    // Render tech
    auto t1 = k1.material->tech;
    auto t2 = k2.material->tech;
    if (t1 != t2) {
        if (t1->getPriority() != t2->getPriority())
            return t1->getPriority() < t2->getPriority();
        // I have no other criteria to sort the tech
        return t1 < t2;
    }
    // Materials Wood vs Dark Wood
    if (k1.material != k2.material)
        return k1.material->getName() < k2.material->getName();
    if (k1.mesh != k2.mesh)
        return k1.mesh->getName() < k2.mesh->getName();
    if (k1.h_render_owner != k2.h_render_owner)
        return k1.h_render_owner.asUnsigned() < k2.h_render_owner.asUnsigned();
    return k1.subgroup_idx < k2.subgroup_idx;
}

void CRenderManager::forceDirty() {
    render_keys.is_dirty = true;
}

void CRenderManager::delRenderKeys(CHandle h_owner) {

    render_keys.deleteByOwner(h_owner);
}

void CRenderManager::addRenderKey(
    CHandle h_comp_render_owner,
    const CRenderMesh* mesh,
    const CMaterial* material,
    uint32_t subgroup_idx) {

    CEntity* e_owner = h_comp_render_owner.getOwner();
    assert(e_owner);

    // Register the basic mesh
    TRenderKey key;
    key.h_render_owner = h_comp_render_owner;
    key.h_aabb = e_owner->get<TCompAbsAABB>();
    key.mesh = mesh;
    key.material = material;
    key.subgroup_idx = subgroup_idx;
    render_keys.addKey(key);

    if (material->castsShadows()) {

        const CMaterial* shadow_mat = nullptr;
        if (material->tech->usesSkin()) {
            shadow_mat = Resources.get("data/materials/shadows_skin.material")->as<CMaterial>();
        }
        else if (material->tech->usesInstancing()) {
            if (material->tech->vs->getVertexDecl()->name == "Pos_x_InstancedPos")
                shadow_mat = Resources.get("data/materials/shadows_grass_instanced.material")->as<CMaterial>();
            else
                shadow_mat = Resources.get("data/materials/shadows_instanced.material")->as<CMaterial>();
        }
        else {
            //if(mesh->getVertexDecl()->name == "PosNUvCvTan" )
            //  shadow_mat = Resources.get("data/materials/shadows_mix.material")->as<CMaterial>();
            //else
            shadow_mat = Resources.get("data/materials/shadows.material")->as<CMaterial>();
        }

        TRenderKey key;
        key.h_render_owner = h_comp_render_owner;
        key.h_aabb = e_owner->get<TCompAbsAABB>();
        key.mesh = mesh;
        key.material = shadow_mat;
        key.subgroup_idx = subgroup_idx;
        render_keys.addKey(key);
    }
}

void CRenderManager::TRenderKey::debugInMenu() {

    char key_name[256];
    std::string mat_name = material->getName();
    snprintf(key_name, 255, "%s %s %s [%d]", material->tech->getCategory().c_str(), mat_name.c_str(), mesh->getName().c_str(), subgroup_idx);
    if (ImGui::TreeNode(key_name)) {

        auto ncmaterial = const_cast<CMaterial*>(material);
        if (ImGui::TreeNode(material->getName().c_str())) {
            ncmaterial->debugInMenu();
            ImGui::TreePop();
        }

        auto ncmesh = const_cast<CRenderMesh*>(mesh);
        ncmesh->debugInMenu();
        ImGui::TreePop();
    }
}

void CRenderManager::debugInMenu() {

    if (ImGui::TreeNode("Render Manager")) {
        auto it = render_keys.begin();
        while (it != render_keys.end()) {
            ImGui::PushID(&(*it));

            // Give the option to delete this specific key
            it->debugInMenu();
            ImGui::SameLine();
            if (ImGui::SmallButton("X"))
                it = render_keys.erase(it);
            else
                ++it;

            ImGui::PopID();
        }

        ImGui::TreePop();
    }
}

void CRenderManager::setEntityCamera(CHandle h_new_entity_camera) {

    h_camera = h_new_entity_camera;
}

void CRenderManager::renderCategory(const char* category_name) {

    PROFILE_FUNCTION(category_name);
    CTraceScoped gpu_scope(category_name);

    uint32_t category_id = getID(category_name);
    render_keys.sortIfRequired();

    // Find the range of itarator matching the requested category 
    auto range = std::equal_range(render_keys.begin(), render_keys.end(), category_id);
    // Range is empty, we are done.
    if (range.first == range.second)
        return;

    // Check if we have culling information from the camera source
    CEntity* e_camera = h_camera;
    const TCompCulling* culling = nullptr;
    if (e_camera)
        culling = e_camera->get<TCompCulling>();
    const TCompCulling::TCullingBits* culling_bits = culling ? &culling->bits : nullptr;

    bool using_skin = false;

    // Convert iterators to raw pointers, using distance from start of the container
    // to avoid accessing the *range.second when range.second == render_keys.end()
    auto offset_to_first = std::distance(render_keys.begin(), range.first);
    auto offset_to_last = std::distance(render_keys.begin(), range.second);
    const TRenderKey* it = render_keys.data() + offset_to_first;
    const TRenderKey* last = render_keys.data() + offset_to_last;

    // To have the prev_it point to something 'valid'
    static TRenderKey null_key;
    const TRenderKey* prev_it = &null_key;

    // For each key in the range of keys
    while (it != last) {
        PROFILE_FUNCTION("Key");

        TCompRender * c_render = it->h_render_owner;

        /* Test if component render is marked as visible */
        if (!c_render->visible) {
            ++it;
            continue;
        }

        // Do the culling
        if (culling_bits) {
            TCompAbsAABB* aabb = it->h_aabb;
            if (aabb) {
                auto idx = it->h_aabb.getExternalIndex();
                if (!culling_bits->test(idx)) {
                    ++it;
                    continue;
                }
            }
        }

        // World asociada a mi objeto
        const TCompTransform* c_transform = it->h_transform;
        cb_object.obj_world = c_transform->asMatrix();
        cb_object.obj_color = c_render->color;
        cb_object.self_color = c_render->self_color;
        cb_object.self_intensity = c_render->self_intensity;
        cb_object.self_opacity = c_render->self_opacity;
        cb_object.updateGPU();

        // Do we have to change the material wrt the prev draw call?
        if (prev_it->material != it->material) {
            it->material->activate();
            using_skin = it->material->tech->usesSkin();
        }

        //Is our material using skinning data?
        if (using_skin) {
            CEntity* e = it->h_render_owner.getOwner();
            assert(e);
            TCompSkeleton* cs = e->get<TCompSkeleton>();
            assert(cs);
            cs->updateCtesBones();
            cs->cb_bones.activate();
        }

        //if (uses_capa) {
        //  CCapaShader* cp = (CCapaShader*) it->tech;
        //  cp->setValues(it->h_render_owner.getOwner());
        //}

        //if (it->usesCustomCtes) 
         // it->material->tech->updateCustomCtes(h_render_owner.getOwner());

        // Has the mesh changed?
        if (prev_it->mesh != it->mesh)
            it->mesh->activate();

        // Do the render
        it->mesh->renderSubMesh(it->subgroup_idx);

        prev_it = it;
        ++it;
    }

}


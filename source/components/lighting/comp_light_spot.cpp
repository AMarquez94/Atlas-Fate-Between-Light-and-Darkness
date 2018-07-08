#include "mcv_platform.h"
#include "comp_light_spot.h"
#include "../comp_transform.h"
#include "render/render_objects.h"    // cb_light
#include "render/texture/texture.h"
#include "render/texture/render_to_texture.h" 
#include "render/render_manager.h" 
#include "render/render_utils.h"
#include "render/gpu_trace.h"
#include "physics/physics_collider.h"

#include "components/comp_aabb.h"
#include "components/comp_culling.h"
#include "components/physics/comp_collider.h"

DECL_OBJ_MANAGER("light_spot", TCompLightSpot);

CRenderMeshInstanced* TCompLightSpot::volume_instance = nullptr;

void TCompLightSpot::debugInMenu() {
    ImGui::Checkbox("Enabled", &isEnabled);
    ImGui::ColorEdit3("Color", &color.x);
    ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.f, 10.f);
    ImGui::DragFloat("Angle", &angle, 0.5f, 1.f, 160.f);
    ImGui::DragFloat("Cut Out", &inner_cut, 0.5f, 1.f, angle);
    ImGui::DragFloat("Range", &range, 0.5f, 1.f, 120.f);
    ImGui::DragFloat("Shadow step", &shadows_step, 0.01f, 0.f, 10.f);
}

void TCompLightSpot::renderDebug() {

    // Render a wire sphere
    auto mesh = Resources.get("data/meshes/UnitCone.mesh")->as<CRenderMesh>();
    renderMesh(mesh, getWorld(), VEC4(1, 1, 1, 1));
}

void TCompLightSpot::load(const json& j, TEntityParseContext& ctx) {

    isEnabled = true;
    TCompCamera::load(j, ctx);

    intensity = j.value("intensity", 1.0f);
    volume_intensity = j.value("volume_intensity", 1.0f);
    color = loadVEC4(j["color"]);

    volume_enabled = j.value("volume", true);
    casts_shadows = j.value("shadows", true);
    num_samples = j.value("n_samples", 80.f);
    angle = j.value("angle", 45.f);
    range = j.value("range", 10.f);
    inner_cut = j.value("inner_cut", angle);
    outer_cut = j.value("outer_cut", angle);

    if (j.count("projector")) {
        std::string projector_name = j.value("projector", "");
        projector = Resources.get(projector_name)->as<CTexture>();
    }
    else {
        projector = Resources.get("data/textures/default_white.dds")->as<CTexture>();
    }

    // Check if we need to allocate a shadow map
    casts_shadows = j.value("casts_shadows", false);
    if (casts_shadows) {
        shadows_step = j.value("shadows_step", shadows_step);
        shadows_resolution = j.value("shadows_resolution", shadows_resolution);
        auto shadowmap_fmt = readFormat(j, "shadows_fmt");
        assert(shadows_resolution > 0);
        shadows_rt = new CRenderToTexture;
        // Make a unique name to have the Resource Manager happy with the unique names for each resource
        char my_name[64];
        sprintf(my_name, "shadow_map_%08x", CHandle(this).asUnsigned());
        bool is_ok = shadows_rt->createRT(my_name, shadows_resolution, shadows_resolution, DXGI_FORMAT_UNKNOWN, shadowmap_fmt);
        assert(is_ok);
    }

    //spotcone = loadMesh("data/meshes/unit_quad_center2.mesh");
    shadows_enabled = casts_shadows;
}

void TCompLightSpot::setColor(const VEC4 & new_color) {

    color = new_color;
}

MAT44 TCompLightSpot::getWorld() {

    TCompTransform* c = get<TCompTransform>();
    if (!c)
        return MAT44::Identity;

    float new_scale = tan(deg2rad(angle * .5f)) * range;
    return MAT44::CreateScale(VEC3(new_scale, new_scale, range)) * c->asMatrix();
}

void TCompLightSpot::update(float dt) {

    TCompTransform * c = get<TCompTransform>();
    if (!c)
        return;

    MAT44 view_proj = getViewProjection();
    //e_owner->sendMsg(TMsgGetCullingViewProj{ &view_proj });
    TCompCulling * c_culling = get<TCompCulling>();
    c_culling->updateFromMatrix(view_proj);

    this->lookAt(c->getPosition(), c->getPosition() + c->getFront(), c->getUp());
    this->setPerspective(deg2rad(angle), 0.1f, range); // might change this znear in the future, hardcoded for clipping purposes.
}

void TCompLightSpot::registerMsgs() {

    DECL_MSG(TCompLightSpot, TMsgEntityCreated, onCreate);
    DECL_MSG(TCompLightSpot, TMsgEntityDestroyed, onDestroy);
    DECL_MSG(TCompLightSpot, TMsgEntitiesGroupCreated, onGroupCreated);
}

// Generate the AABB for the spotlight
void TCompLightSpot::onCreate(const TMsgEntityCreated& msg) {

    TCompAbsAABB * c_my_aabb = get<TCompAbsAABB>();
    TCompLocalAABB * c_my_aabb_local = get<TCompLocalAABB>();
    TCompCollider * c_my_collider = get<TCompCollider>();

    if (c_my_collider && c_my_collider->config->shape) {
        physx::PxConvexMeshGeometry colliderMesh;
        c_my_collider->config->shape->getConvexMeshGeometry(colliderMesh);
        physx::PxBounds3 bounds = colliderMesh.convexMesh->getLocalBounds();
        VEC3 extents = PXVEC3_TO_VEC3(bounds.getExtents());

        c_my_aabb->Center = VEC3::Zero;
        c_my_aabb->Extents = extents;
    }
    else if (c_my_aabb && c_my_aabb_local) {

        TCompTransform* c_my_transform = get<TCompTransform>();
        VEC3 c_my_center = c_my_transform->getPosition() + range * .5f * c_my_transform->getFront();

        c_my_aabb->Extents = VEC3(tan(deg2rad(angle / 2)) * range, tan(deg2rad(angle / 2)) * range, range *.5f );
        c_my_aabb_local->Extents = VEC3(tan(deg2rad(angle / 2)) * range, tan(deg2rad(angle / 2)) * range, range *.5f);

        c_my_aabb->Center = VEC3(0, 0, range * .5f);
        c_my_aabb_local->Center = VEC3(0, 0, range * .5f);
    }

    CEntity* e = CHandle(this).getOwner();

    // Add a particle component
    CHandle h_comp = getObjectManager<TCompCulling>()->createHandle();
    e->set(h_comp.getType(), h_comp);

    //for (int i = 0; i < num_samples; i++) {
    //    EngineInstancing.addInstance("data/meshes/quad_volume.instanced_mesh", MAT44::Identity);
    //}
}

void TCompLightSpot::onDestroy(const TMsgEntityDestroyed & msg) {

}

void TCompLightSpot::onGroupCreated(const TMsgEntitiesGroupCreated & msg)
{
    TCompCulling* myCulling = get<TCompCulling>();
    if (myCulling == nullptr) {
        TCompAbsAABB * c_my_aabb = get<TCompAbsAABB>();
        TCompLocalAABB * c_my_aabb_local = get<TCompLocalAABB>();
        TCompCollider * c_my_collider = get<TCompCollider>();

        if (c_my_collider && c_my_collider->config->shape) {
            physx::PxConvexMeshGeometry colliderMesh;
            c_my_collider->config->shape->getConvexMeshGeometry(colliderMesh);
            physx::PxBounds3 bounds = colliderMesh.convexMesh->getLocalBounds();
            VEC3 extents = PXVEC3_TO_VEC3(bounds.getExtents());

            c_my_aabb->Center = VEC3::Zero;
            c_my_aabb->Extents = extents;
        }
        else if (c_my_aabb && c_my_aabb_local) {

            TCompTransform* c_my_transform = get<TCompTransform>();
            VEC3 c_my_center = c_my_transform->getPosition() + range * .5f * c_my_transform->getFront();

            c_my_aabb->Extents = VEC3(tan(deg2rad(angle / 2)) * range, tan(deg2rad(angle / 2)) * range, range *.5f);
            c_my_aabb_local->Extents = VEC3(tan(deg2rad(angle / 2)) * range, tan(deg2rad(angle / 2)) * range, range *.5f);

            c_my_aabb->Center = VEC3(0, 0, range * .5f);
            c_my_aabb_local->Center = VEC3(0, 0, range * .5f);
        }

        CEntity* e = CHandle(this).getOwner();

        // Add a particle component
        CHandle h_comp = getObjectManager<TCompCulling>()->createHandle();
        e->set(h_comp.getType(), h_comp);
    }
}

void TCompLightSpot::activate() {

    TCompTransform* c = get<TCompTransform>();
    if (!c || !isEnabled || cull_enabled)
        return;

    projector->activate(TS_LIGHT_PROJECTOR);
    // To avoid converting the range -1..1 to 0..1 in the shader
    // we concatenate the view_proj with a matrix to apply this offset
    MAT44 mtx_offset = MAT44::CreateScale(VEC3(0.5f, -0.5f, 1.0f)) * MAT44::CreateTranslation(VEC3(0.5f, 0.5f, 0.0f));

    float spot_angle = cos(deg2rad(angle * .5f));
    cb_light.light_color = color;
    cb_light.light_intensity = intensity;
    cb_light.light_pos = c->getPosition();
    cb_light.light_radius = range * c->getScale();
    cb_light.light_view_proj_offset = getViewProjection() * mtx_offset;
    cb_light.light_angle = spot_angle;
    cb_light.light_direction = VEC4(c->getFront().x, c->getFront().y, c->getFront().z, 1);
    cb_light.light_inner_cut = cos(deg2rad(Clamp(inner_cut, 0.f, angle) * .5f));
    cb_light.light_outer_cut = spot_angle;
    cb_light.far_atten = 0.98f;
    cb_light.inner_atten = 0.9f;

    // If we have a ZTexture, it's the time to activate it
    if (shadows_rt) {

        cb_light.light_shadows_inverse_resolution = 1.0f / (float)shadows_rt->getWidth();
        cb_light.light_shadows_step = shadows_step;
        cb_light.light_shadows_step_with_inv_res = shadows_step / (float)shadows_rt->getWidth();
        cb_light.light_radius = range;

        assert(shadows_rt->getZTexture());
        shadows_rt->getZTexture()->activate(TS_LIGHT_SHADOW_MAP);
    }

    cb_light.updateGPU();
}

// Dirty way of computing volumetric lights on CPU.
// Update this in the future with a vertex shader improved version.
void TCompLightSpot::generateVolume() {

    if (!isEnabled || cull_enabled || !volume_enabled)
        return;
    
    CEntity* eCurrentCamera = Engine.getCameras().getOutputCamera();
    TCompCamera* camera = eCurrentCamera->get< TCompCamera >();

    TCompTransform * c_transform = get<TCompTransform>();
    VEC3 cpos = c_transform->getPosition();

    //num_samples = (int)(range / 0.2f);
    float spot_angle = cos(deg2rad(angle * .5f));
    float spot_extent = (getZFar() - getZNear()) * .5f;
    float spot_offset = (getZFar() - getZNear()) / num_samples;
    VEC3 midpos = c_transform->getPosition() + c_transform->getFront() * (getZFar() - getZNear()) * .5f;
    VEC3 endpos = midpos + spot_extent * camera->getFront();

    MAT44 mtx_offset = MAT44::CreateScale(VEC3(0.5f, -0.5f, 1.0f)) * MAT44::CreateTranslation(VEC3(0.5f, 0.5f, 0.0f));
    MAT44 mtx_viewproj_offset = getViewProjection() * mtx_offset;
    
    std::vector<TInstanceLight> volume_instances;
    volume_instances.reserve(num_samples);

    for (int i = 0; i < num_samples; i++) {

        VEC3 plane_pos = endpos - camera->getFront() * spot_offset * i;
        MAT44 bb = MAT44::CreateWorld(plane_pos, -camera->getUp(), -camera->getFront());
        MAT44 sc = MAT44::CreateScale(range, range, range*2);
        MAT44 res = sc * bb;

        TInstanceLight t_struct = { res, VEC4(cpos.x, cpos.y, cpos.z, 1)
            ,VEC4(c_transform->getFront().x, c_transform->getFront().y, c_transform->getFront().z, 1)
            ,VEC4(spot_angle, cos(deg2rad(Clamp(inner_cut, 0.f, angle) * .5f)), spot_angle, volume_intensity), mtx_viewproj_offset };
        volume_instances.push_back(t_struct);
    }

    TCompLightSpot::volume_instance->setInstancesData(volume_instances.data(), volume_instances.size(), sizeof(TInstanceLight));
    // Activate tech for the light dir 
    auto technique2 = Resources.get("pbr_instanced_volume.tech")->as<CRenderTechnique>();
    technique2->activate();

    CRenderManager::get().renderCategory("pbr_volume");
}

void TCompLightSpot::cullFrame() {

    CEntity* e_camera = EngineRender.getMainCamera();
    assert(e_camera);

    const TCompCulling* culling = e_camera->get<TCompCulling>();
    const TCompCulling::TCullingBits* culling_bits = culling ? &culling->bits : nullptr;

    // Do the culling
    if (culling_bits) {
        TCompAbsAABB* aabb = get<TCompAbsAABB>();
        if (aabb) {
            CHandle h = aabb;
            auto idx = h.getExternalIndex();
            if (!culling_bits->test(idx)) {
                CEntity* e = CHandle(this).getOwner();
                cull_enabled = true;
                return;
            }
        }
    }

    cull_enabled = false;
}

// ------------------------------------------------------
void TCompLightSpot::generateShadowMap() {


    if (cull_enabled || !shadows_rt || !shadows_enabled || !isEnabled)
        return;

    // In this slot is where we activate the render targets that we are going
    // to update now. You can't be active as texture and render target at the
    // same time
    CTexture::setNullTexture(TS_LIGHT_SHADOW_MAP);

    CTraceScoped gpu_scope(shadows_rt->getName().c_str());
    shadows_rt->activateRT();

    {
        PROFILE_FUNCTION("Clear&SetCommonCtes");
        shadows_rt->clearZ();
        // We are going to render the scene from the light position & orientation
        activateCamera(*this, shadows_rt->getWidth(), shadows_rt->getHeight());
    }

    CRenderManager::get().setEntityCamera(CHandle(this).getOwner());
    CRenderManager::get().renderCategory("shadows");
}

bool TCompLightSpot::isCulled() const {

    return cull_enabled;
}

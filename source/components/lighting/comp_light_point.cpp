#include "mcv_platform.h"
#include "comp_light_point.h"
#include "render/render_manager.h" 
#include "render/render_objects.h" 
#include "components/comp_transform.h"
#include "components/comp_culling.h"
#include "components/comp_aabb.h"
#include "physics/physics_collider.h"
DECL_OBJ_MANAGER("light_point", TCompLightPoint);

DXGI_FORMAT readFormat(const json& j, const std::string& label);

// -------------------------------------------------
void TCompLightPoint::debugInMenu() {

    ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.f, 10.f);
    ImGui::ColorEdit3("Color", &color.x);
    ImGui::DragFloat("Inner Cut", &inner_cut, 0.01f, 0.f, 100.f);
    ImGui::DragFloat("Outer Cut", &outer_cut, 0.01f, 0.f, 100.f);
    ImGui::Checkbox("Enabled", &isEnabled);
}

MAT44 TCompLightPoint::getWorld() {

    TCompTransform* c = get<TCompTransform>();
    if (!c)
        return MAT44::Identity;

    return MAT44::CreateScale(outer_cut) * c->asMatrix();
}

// -------------------------------------------------
void TCompLightPoint::renderDebug() {

    // Render a wire sphere
    auto mesh = Resources.get("data/meshes/UnitSphere.mesh")->as<CRenderMesh>();
    renderMesh(mesh, getWorld(), VEC4(1, 1, 1, 1));
}

// -------------------------------------------------
void TCompLightPoint::load(const json& j, TEntityParseContext& ctx) {

    if (j.count("color"))
        color = loadVEC4(j["color"]);
    intensity = j.value("intensity", intensity);
    inner_cut = j.value("inner_cut", inner_cut) + 0.01;
    outer_cut = j.value("outer_cut", outer_cut);

    if (j.count("projector")) {
        std::string projector_name = j.value("projector", "");
        projector = Resources.get(projector_name)->as<CTexture>();
    }
    else {
        projector = Resources.get("data/textures/default_white.dds")->as<CTexture>();
    }

    isEnabled = true;
}

void TCompLightPoint::registerMsgs() {

    DECL_MSG(TCompLightPoint, TMsgEntityCreated, onCreate);
    DECL_MSG(TCompLightPoint, TMsgEntitiesGroupCreated, onGroupCreated);
}

// Generate the AABB for the spotlight
void TCompLightPoint::onCreate(const TMsgEntityCreated& msg) {

    TCompAbsAABB * c_my_aabb = get<TCompAbsAABB>();
    TCompLocalAABB * c_my_aabb_local = get<TCompLocalAABB>();

    float half_radius = outer_cut;
    c_my_aabb->Extents = VEC3(half_radius, half_radius, half_radius);
    c_my_aabb_local->Extents = VEC3(half_radius, half_radius, half_radius);

    c_my_aabb->Center = VEC3(0, 0, 0);
    c_my_aabb_local->Center = VEC3(0, 0, 0);

    CEntity* e = CHandle(this).getOwner();
    CHandle h_comp = getObjectManager<TCompCulling>()->createHandle();
    e->set(h_comp.getType(), h_comp);
}

void TCompLightPoint::onGroupCreated(const TMsgEntitiesGroupCreated & msg)
{
    TCompAbsAABB * c_my_aabb = get<TCompAbsAABB>();
    TCompLocalAABB * c_my_aabb_local = get<TCompLocalAABB>();
    TCompCulling * culling = get<TCompCulling>();

    if (culling == nullptr) {

        float half_radius = outer_cut;
        c_my_aabb->Extents = VEC3(half_radius, half_radius, half_radius);
        c_my_aabb_local->Extents = VEC3(half_radius, half_radius, half_radius);

        c_my_aabb->Center = VEC3(0, 0, 0);
        c_my_aabb_local->Center = VEC3(0, 0, 0);

        CEntity* e = CHandle(this).getOwner();
        CHandle h_comp = getObjectManager<TCompCulling>()->createHandle();
        e->set(h_comp.getType(), h_comp);
    }
}

void TCompLightPoint::cullFrame() {

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

// -------------------------------------------------
// Updates the Shader Cte Light with MY information
void TCompLightPoint::activate() {

    TCompTransform* c = get<TCompTransform>();
    if (cull_enabled || !c || !isEnabled)
        return;

    projector->activate(TS_LIGHT_PROJECTOR);

    cb_light.light_color = color;
    cb_light.light_intensity = intensity;
    cb_light.light_pos = c->getPosition();
    cb_light.light_radius = outer_cut *c->getScale();
    cb_light.far_atten = 1.f;
    cb_light.inner_atten = inner_cut / outer_cut;
    cb_light.light_view_proj_offset = MAT44::Identity;
    cb_light.light_angle = 0;
    cb_light.updateGPU();
}

bool TCompLightPoint::isCulled() const {

    return cull_enabled;
}


VEC4 TCompLightPoint::getColor() const {

    return color;
}

void TCompLightPoint::setColor(VEC4 new_color) {

    color = new_color;
}
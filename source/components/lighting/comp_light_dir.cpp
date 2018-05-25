#include "mcv_platform.h"
#include "comp_light_dir.h"
#include "../comp_transform.h"
#include "render/render_objects.h"    // cb_light
#include "render/texture/texture.h"
#include "ctes.h"                     // texture slots
#include "render/texture/render_to_texture.h" 
#include "render/render_manager.h" 
#include "render/render_utils.h"
#include "render/gpu_trace.h"

DECL_OBJ_MANAGER("light_dir", TCompLightDir);

void TCompLightDir::debugInMenu() {
    TCompCamera::debugInMenu();
    ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.f, 10.f);
    ImGui::ColorEdit3("Color", &color.x);
    ImGui::Checkbox("Enabled", &isEnabled);
}

void TCompLightDir::renderDebug() {

    TCompCamera::renderDebug();
}

void TCompLightDir::load(const json& j, TEntityParseContext& ctx) {

    isEnabled = true;
    TCompCamera::load(j, ctx);

    intensity = j.value("intensity", 1.0f);
    color = loadVEC4(j["color"]);

    casts_shadows = j.value("shadows", true);
    angle = j.value("angle", 45.f);
    range = j.value("range", 10.f);

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
        bool is_ok = shadows_rt->createRT(my_name, shadows_resolution, shadows_resolution, DXGI_FORMAT_R8G8B8A8_UNORM, shadowmap_fmt);
        assert(is_ok);
    }

    shadows_enabled = casts_shadows;
}


void TCompLightDir::update(float dt) {

    TCompTransform * c = get<TCompTransform>();
    if (!c)
        return;

    this->lookAt(c->getPosition(), c->getPosition() + c->getFront(), c->getUp());
}

void TCompLightDir::registerMsgs() {

    DECL_MSG(TCompLightDir, TMsgEntityCreated, onCreate);
    DECL_MSG(TCompLightDir, TMsgEntityDestroyed, onDestroy);
}

void TCompLightDir::onCreate(const TMsgEntityCreated& msg) {

}

void TCompLightDir::onDestroy(const TMsgEntityDestroyed & msg) {

}

void TCompLightDir::activate() {

    TCompTransform * c = get<TCompTransform>();
    if (!c || !isEnabled)
        return;

    // Handling the projector
    projector->activate(TS_LIGHT_PROJECTOR);

    // To avoid converting the range -1..1 to 0..1 in the shader
    // we concatenate the view_proj with a matrix to apply this offset
    MAT44 mtx_offset = MAT44::CreateScale(VEC3(0.5f, -0.5f, 1.0f))
        * MAT44::CreateTranslation(VEC3(0.5f, 0.5f, 0.0f));

    cb_light.light_color = color;
    cb_light.light_intensity = intensity;
    cb_light.light_pos = c->getPosition();
    cb_light.light_view_proj_offset = getViewProjection() * mtx_offset;
    cb_light.light_radius = getZFar();
    cb_light.light_angle = 0;

    // If we have a ZTexture, it's the time to activate it
    if (shadows_rt) {

        cb_light.light_shadows_inverse_resolution = 1.0f / (float)shadows_rt->getWidth();
        cb_light.light_shadows_step = shadows_step;
        cb_light.light_shadows_step_with_inv_res = shadows_step / (float)shadows_rt->getWidth();
        //cb_light.light_radius = 0.f;

        assert(shadows_rt->getZTexture());
        shadows_rt->getZTexture()->activate(TS_LIGHT_SHADOW_MAP);
    }

    cb_light.updateGPU();
}


// ------------------------------------------------------
void TCompLightDir::generateShadowMap() {

    if (!shadows_rt || !shadows_enabled || !isEnabled)
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



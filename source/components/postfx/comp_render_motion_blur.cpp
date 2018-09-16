#include "mcv_platform.h"
#include "comp_render_motion_blur.h"
#include "resources/resources_manager.h"
#include "render/texture/render_to_texture.h"

DECL_OBJ_MANAGER("motion_blur", TCompRenderMotionBlur);

// ---------------------
TCompRenderMotionBlur::TCompRenderMotionBlur()
{

}

TCompRenderMotionBlur::~TCompRenderMotionBlur() {

}

void TCompRenderMotionBlur::debugInMenu() {

    ImGui::Checkbox("Enabled", &enabled);
    ImGui::DragFloat("Blur Samples", &cb_blur.mblur_blur_samples, 0.1f, 0.f, 1000.f);
    ImGui::DragFloat("Blur speed", &cb_blur.mblur_blur_speed, 0.1f, 0.f, 300.f);
    ImGui::DragFloat("Samples", &cb_blur.mblur_samples, 0.1f, 0.f, 1000.f);
}

void TCompRenderMotionBlur::load(const json& j, TEntityParseContext& ctx) {

    enabled = j.value("enabled", true);
    cb_blur.mblur_blur_samples = j.value("blur_samples", 8.f);
    cb_blur.mblur_blur_speed = j.value("blur_speed", 0.35f);
    cb_blur.mblur_samples = j.value("samples", 8.f);

    tech = Resources.get("postfx_motion_blur.tech")->as<CRenderTechnique>();
    mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();

    rt = new CRenderToTexture;
    char rt_name[64];
    sprintf(rt_name, "RT_MotionBlur_%08x", CHandle(this).asUnsigned());
    bool is_ok = rt->createRT(rt_name, Render.width, Render.height, DXGI_FORMAT_R8G8B8A8_UNORM);
    assert(is_ok);
}

CTexture* TCompRenderMotionBlur::apply(CTexture* in_texture) {

    if (!enabled)
        return in_texture;

    CTraceScoped scope("TCompMotionBlur");
    cb_blur.updateGPU();
    cb_blur.activate();

    rt->activateRT();
    in_texture->activate(TS_ALBEDO);

    tech->activate();
    mesh->activateAndRender();

    return rt;
}
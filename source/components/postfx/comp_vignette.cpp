#include "mcv_platform.h"
#include "comp_vignette.h"
#include "render/texture/render_to_texture.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("vignette", TCompVignette);

// ---------------------
void TCompVignette::debugInMenu() {

    ImGui::Checkbox("Enabled", &enabled);
    ImGui::DragFloat("Amount", &amount, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Softness Amount", &softness_amount, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Contrast", &contrast, 0.01f, 0.0f, 3.0f);
    //ImGui::DragFloat("Brightness", &brightness, 0.01f, 0.0f, 3.0f);
}

void TCompVignette::load(const json& j, TEntityParseContext& ctx) {

    int xres = Render.width;
    int yres = Render.height;

    enabled = j.value("enabled", true);
    amount = j.value("amount", 0.32f);
    softness_amount = j.value("softness_amount", 0.52f);
    contrast = j.value("contrast", 1.05f);
    brightness = j.value("brightness", 0.52f);

    if (!rt) {
        rt = new CRenderToTexture;
        // Create a unique name for the rt 
        char rt_name[64];
        sprintf(rt_name, "Vignette_%08x", CHandle(this).asUnsigned());
        bool is_ok = rt->createRT(rt_name, xres, yres, DXGI_FORMAT_B8G8R8A8_UNORM);
        assert(is_ok);
    }

    //std::string lut_name = j.value("lut", "");
    //lut1 = Resources.get(lut_name)->as<CTexture>();

    tech = Resources.get("postfx_vignette.tech")->as<CRenderTechnique>();
    mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
}

CTexture* TCompVignette::apply(CTexture* in_texture) {

    if (!enabled)
        return in_texture;

    CTraceScoped scope("TCompVignette");
    cb_postfx.postfx_vignette = amount;
    cb_postfx.postfx_vignette_softness = softness_amount;
    cb_postfx.postfx_contrast = contrast;
    cb_postfx.postfx_brightness = brightness;

    cb_globals.global_shared_fx_amount = amount;
    cb_globals.updateGPU();
    cb_postfx.updateGPU();

    rt->activateRT();
    in_texture->activate(TS_ALBEDO);

    tech->activate();
    mesh->activateAndRender();

    return rt;
}
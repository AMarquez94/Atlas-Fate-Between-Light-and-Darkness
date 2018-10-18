#include "mcv_platform.h"
#include "comp_chrom_aberration.h"
#include "render/texture/render_to_texture.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("chromatic_aberration", TCompChromaticAberration);

// ---------------------
void TCompChromaticAberration::debugInMenu() {

    ImGui::Checkbox("Enabled", &enabled);
    ImGui::DragFloat("Amount", &amount, 0.01f, 0.0f, 4.0f);
    ImGui::DragFloat("Block Amount", &block_amount, 0.01f, 0.0f, 4.0f);
    ImGui::DragFloat("Scan Jitter", &scanline_jitter, 0.01f, 0.0f, 4.0f);
    ImGui::DragFloat("Scan Drift", &scanline_drift, 0.01f, 0.0f, 4.0f);
}

void TCompChromaticAberration::load(const json& j, TEntityParseContext& ctx) {

    enabled = j.value("enabled", true);
    amount = j.value("amount", 1.0f);
    scanline_jitter = j.value("jitter", 1.190f);
    scanline_drift = j.value("drift", 0.520f);
    block_amount = j.value("block", 0.04f);

    int xres = Render.width;
    int yres = Render.height;

    if (!rt) {
        rt = new CRenderToTexture;
        // Create a unique name for the rt 
        char rt_name[64];
        sprintf(rt_name, "Chromatic_Aberration_%08x", CHandle(this).asUnsigned());
        bool is_ok = rt->createRT(rt_name, xres, yres, DXGI_FORMAT_B8G8R8A8_UNORM);
        assert(is_ok);
    }

    tech = Resources.get("postfx_chromatic_aberration.tech")->as<CRenderTechnique>();
    mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();

    cb_postfx.postfx_ca_amount = 0.09f;

}

CTexture* TCompChromaticAberration::apply(CTexture* in_texture) {

    float threshold = Clamp(1.0f - scanline_jitter * 1.2f, 0.f, 1.f);
    float disperssion = pow(scanline_jitter, 3) * 0.05f;
    cb_postfx.postfx_block_random = urand(-1, 1);
    cb_postfx.postfx_block_amount = block_amount;
    //cb_postfx.postfx_scan_amount = amount;
    cb_postfx.postfx_scan_drift = VEC2(scanline_drift * 0.04f, cb_globals.global_world_time * 600);
    cb_postfx.postfx_scan_jitter = VEC2(disperssion, threshold);
    cb_postfx.updateGPU();
    cb_postfx.activate();

    if (!enabled)
        return in_texture;

    CTraceScoped scope("TCompChromaticAberration");
    cb_globals.updateGPU();

    rt->activateRT();
    in_texture->activate(TS_ALBEDO);
    tech->activate();
    mesh->activateAndRender();

    return rt;
}
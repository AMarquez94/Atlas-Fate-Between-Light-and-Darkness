#include "mcv_platform.h"
#include "comp_render_outlines.h"
#include "render/texture/render_to_texture.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"

extern ID3D11ShaderResourceView* depth_shader_resource_view;

DECL_OBJ_MANAGER("render_outlines", TCompRenderOutlines);

// ---------------------
void TCompRenderOutlines::debugInMenu() {

    ImGui::Checkbox("Enabled", &enabled);
    ImGui::DragFloat("Amount", &amount, 0.01f, 0.0f, 1.0f);
}

void TCompRenderOutlines::load(const json& j, TEntityParseContext& ctx) {

    enabled = j.value("enabled", true);
    amount = j.value("amount", 1.0f);
    int xres = Render.width;
    int yres = Render.height;

    if (!post_rt) {
        post_rt = new CRenderToTexture;
        // Create a unique name for the rt 
        char rt_name[64];
        sprintf(rt_name, "Fog_%08x", CHandle(this).asUnsigned());
        bool is_ok = post_rt->createRT(rt_name, xres, yres, DXGI_FORMAT_B8G8R8A8_UNORM);
        assert(is_ok);
    }

    tech = Resources.get("outlines.tech")->as<CRenderTechnique>();
    post_tech = Resources.get("postfx_scanner.tech")->as<CRenderTechnique>();
    mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
}

CTexture* TCompRenderOutlines::apply(CTexture* in_texture) {

    if (!enabled)
        return in_texture;

    CTraceScoped scope("TCompRenderOutlines");

    // Upload to the GPU the how much visibe is the effect (defaults to 100%)
    cb_globals.global_shared_fx_amount = amount;
    cb_globals.updateGPU();

    // Disable the ZBuffer
    CRenderToTexture* rt = CRenderToTexture::getCurrentRT();
    ID3D11RenderTargetView* rtv = rt->getRenderTargetView();
    Render.ctx->OMSetRenderTargets(1, &rtv, nullptr);

    // Activate the depth stencil buffer as texture 0
    Render.ctx->PSSetShaderResources(TS_ALBEDO, 1, &Render.depth_shader_resource_view);
    tech->activate();
    mesh->activateAndRender();

    // Restore the current render target as it was
    CTexture::setNullTexture(TS_ALBEDO);
    rt->activateRT();

    // Compute the fullscreen post process shockwave
    post_rt->activateRT();
    in_texture->activate(TS_ALBEDO);
    //in_outline_albedo->activate(TS_EMISSIVE);

    post_tech->activate();
    mesh->activateAndRender();

    return post_rt;
}
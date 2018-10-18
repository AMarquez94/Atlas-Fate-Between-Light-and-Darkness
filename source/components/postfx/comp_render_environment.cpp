#include "mcv_platform.h"
#include "comp_render_environment.h"
#include "render/texture/render_to_texture.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"

extern ID3D11ShaderResourceView* depth_shader_resource_view;

DECL_OBJ_MANAGER("render_environment", TCompRenderEnvironment);

// ---------------------
void TCompRenderEnvironment::debugInMenu() {

    ImGui::Checkbox("Enabled", &enabled);
    ImGui::DragFloat("Amount", &amount, 0.01f, 0.0f, 1.0f);
}

void TCompRenderEnvironment::load(const json& j, TEntityParseContext& ctx) {

    enabled = j.value("enabled", true);
    amount = j.value("amount", 1.0f);
    int xres = Render.width;
    int yres = Render.height;

    if (!rt) {
        rt = new CRenderToTexture;
        // Create a unique name for the rt 
        char rt_name[64];
        sprintf(rt_name, "PostEnvironment_%08x", CHandle(this).asUnsigned());
        bool is_ok = rt->createRT(rt_name, xres, yres, DXGI_FORMAT_B8G8R8A8_UNORM);
        assert(is_ok);
    }

    tech = Resources.get("postfx_environment.tech")->as<CRenderTechnique>();
    mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
}

CTexture* TCompRenderEnvironment::apply(CTexture* in_texture) {

    if (!enabled)
        return in_texture;

    CTraceScoped scope("TCompRenderEnvironment");

    rt->activateRT();
    Render.ctx->PSSetShaderResources(TS_ALBEDO, 1, &Render.depth_shader_resource_view);
    in_texture->activate(TS_EMISSIVE);

    tech->activate();
    mesh->activateAndRender();

    return rt;
}
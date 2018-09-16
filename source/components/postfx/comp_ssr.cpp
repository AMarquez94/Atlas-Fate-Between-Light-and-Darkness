#include "mcv_platform.h"
#include "comp_ssr.h"
#include "render/texture/render_to_texture.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("ssr", TCompSSR);

// ---------------------
void TCompSSR::debugInMenu() {

    ImGui::Checkbox("Enabled", &enabled);
}

void TCompSSR::load(const json& j, TEntityParseContext& ctx) {

    int xres = Render.width;
    int yres = Render.height;

    enabled = j.value("enabled", true);
    amount = j.value("amount", 1.0f);

    if (!rt) {
        rt = new CRenderToTexture;
        // Create a unique name for the rt 
        char rt_name[64];
        sprintf(rt_name, "SSR_%08x", CHandle(this).asUnsigned());
        bool is_ok = rt->createRT(rt_name, xres, yres, DXGI_FORMAT_B8G8R8A8_UNORM);
        assert(is_ok);
    }

    tech = Resources.get("postfx_ssr.tech")->as<CRenderTechnique>();
    mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
}

CTexture* TCompSSR::apply(CTexture* in_texture) {

    if (!enabled)
        return in_texture;

    CTraceScoped scope("TCompSSR");
    cb_globals.updateGPU();

    rt->activateRT();
    in_texture->activate(TS_ALBEDO);

    tech->activate();
    mesh->activateAndRender();

    return rt;
}
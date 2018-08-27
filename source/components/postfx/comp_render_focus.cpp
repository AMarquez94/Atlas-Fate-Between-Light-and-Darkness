#include "mcv_platform.h"
#include "comp_render_focus.h"
#include "resources/resources_manager.h"
#include "render/texture/render_to_texture.h"

DECL_OBJ_MANAGER("render_focus", TCompRenderFocus);

// ---------------------
TCompRenderFocus::TCompRenderFocus()
    : cte_focus("Focus")
{
    bool is_ok = cte_focus.create(CB_FOCUS);
    assert(is_ok);
}

TCompRenderFocus::~TCompRenderFocus() {

    cte_focus.destroy();
}

void TCompRenderFocus::debugInMenu() {

    ImGui::Checkbox("Enabled", &enabled);
    ImGui::DragFloat("Z Center In Focus", &cte_focus.focus_z_center_in_focus, 0.1f, 0.f, 1000.f);
    ImGui::DragFloat("Margin In Focus", &cte_focus.focus_z_margin_in_focus, 0.1f, 0.f, 300.f);
    ImGui::DragFloat("Transition Distance", &cte_focus.focus_transition_distance, 0.1f, 0.f, 1000.f);
    ImGui::DragFloat("Focus Modifier", &cte_focus.focus_modifier, 0.1f, 0.f, 3.0f);
}

void TCompRenderFocus::load(const json& j, TEntityParseContext& ctx) {

    enabled = j.value("enabled", true);
    cte_focus.focus_z_center_in_focus = j.value("z_center_in_focus", 30.f);
    cte_focus.focus_z_margin_in_focus = j.value("z_margin_in_focus", 0.f);
    cte_focus.focus_transition_distance = j.value("transition_distance", 15.f);
    cte_focus.focus_modifier = j.value("focus_modifier", 1.f);
    tech = Resources.get("focus.tech")->as<CRenderTechnique>();
    mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();

    // with the first use, init with the input resolution
    rt = new CRenderToTexture;
    bool is_ok = rt->createRT("RT_Focus", Render.width, Render.height, DXGI_FORMAT_R8G8B8A8_UNORM);
    assert(is_ok);
}

CTexture* TCompRenderFocus::apply(CTexture* focus_texture, CTexture* blur_texture) {

    if (!enabled)
        return focus_texture;

    assert(rt);

    cte_focus.updateGPU();
    cte_focus.activate();

    assert(mesh);
    assert(tech);
    assert(focus_texture);
    assert(blur_texture);

    rt->activateRT();
    focus_texture->activate(TS_ALBEDO);
    blur_texture->activate(TS_ALBEDO1);
    tech->activate();
    mesh->activateAndRender();

    return rt;
}
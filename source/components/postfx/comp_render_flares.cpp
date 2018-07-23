#include "mcv_platform.h"
#include "comp_render_flares.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("render_flares", TCompRenderFlares);

// ---------------------
void TCompRenderFlares::debugInMenu() {

    TCompRenderBlur::debugInMenu();
	ImGui::DragFloat("Amount", &amount, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Radius", &radius, 0.01f, 0.0f, 10.0f);
}

void TCompRenderFlares::load(const json& j, TEntityParseContext& ctx) {

    TCompRenderBlur::load(j, ctx);
	enabled = j.value("enabled", enabled);
	amount = j.value("amount", amount);
	radius = j.value("radius", radius);

	xres = Render.width;
	yres = Render.height;

	static int g_counter = 0;
	rt_output = new CRenderToTexture();
	char rt_name[64];
	sprintf(rt_name, "Flares_%02d", g_counter++);
	bool is_ok = rt_output->createRT(rt_name, xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	assert(is_ok);

	tech = Resources.get("postfx_flare.tech")->as<CRenderTechnique>();
	mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
}

CTexture* TCompRenderFlares::apply(CTexture* in_color, CTexture * in_lights) {

	if (!enabled)
		return in_color;

	CTraceScoped scope("TCompFlares");

	rt_output->activateRT();
    in_color->activate(TS_ALBEDO);
    in_lights->activate(TS_EMISSIVE);

	tech->activate();
	mesh->activateAndRender();

	return rt_output;
}
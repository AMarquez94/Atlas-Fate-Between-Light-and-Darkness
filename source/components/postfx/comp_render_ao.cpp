#include "mcv_platform.h"
#include "comp_render_ao.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("render_ao", TCompRenderAO);

// ---------------------
void TCompRenderAO::debugInMenu() {
    TCompRenderBlur::debugInMenu();
	ImGui::Checkbox("Enabled", &enabled);
	ImGui::DragFloat("Amount", &amount, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Radius", &radius, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("ZRange Discard", &zrange_discard, 0.001f, 0.0f, 0.1f);
	ImGui::DragFloat("Amount Spread", &amount_spreading, 0.01f, 0.0f, 1.0f);
}

void TCompRenderAO::load(const json& j, TEntityParseContext& ctx) {
    TCompRenderBlur::load(j, ctx);
	enabled = j.value("enabled", enabled);
	amount = j.value("amount", amount);
	radius = j.value("radius", radius);
	zrange_discard = j.value("zrange_discard", zrange_discard);
	amount_spreading = j.value("amount_spreading", amount_spreading);

	xres = Render.width;
	yres = Render.height;

	static int g_counter = 0;
	rt_output = new CRenderToTexture();
	char rt_name[64];
	sprintf(rt_name, "AO_%02d", g_counter++);
	bool is_ok = rt_output->createRT(rt_name, xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	assert(is_ok);

	white = Resources.get("data/textures/white.dds")->as<CTexture>();
	tech = Resources.get("ao.tech")->as<CRenderTechnique>();
	mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
}

const CTexture* TCompRenderAO::compute(CTexture* linear_depth_texture) {
	if (!enabled)
		return white;
	CTraceScoped scope("AO");

	rt_output->activateRT();
	linear_depth_texture->activate(TS_DEFERRED_LINEAR_DEPTH);

	cb_globals.global_shared_fx_amount = amount;
	cb_globals.global_shared_fx_val1 = radius;
	cb_globals.global_shared_fx_val2 = zrange_discard;
	cb_globals.global_shared_fx_val3 = 10 * (1.0f - amount_spreading);
	cb_globals.updateGPU();

	tech->activate();
	mesh->activateAndRender();

    // Blur the highlights
    CTexture* output = apply(rt_output);

	ID3D11RenderTargetView* null_rt = nullptr;
	Render.ctx->OMSetRenderTargets(1, &null_rt, nullptr);

	return output;
}
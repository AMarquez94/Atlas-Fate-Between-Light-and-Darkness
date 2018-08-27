#include "mcv_platform.h"
#include "comp_render_flares.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"
#include "render/blur_step.h"

DECL_OBJ_MANAGER("render_flares", TCompRenderFlares);

// ---------------------
void TCompRenderFlares::debugInMenu() {

    TCompRenderBlur::debugInMenu();
}

void TCompRenderFlares::load(const json& j, TEntityParseContext& ctx) {

    TCompRenderBlur::load(j, ctx);
	enabled = j.value("enabled", enabled);

	xres = Render.width;
	yres = Render.height;

	static int g_counter = 0;
	rt_output = new CRenderToTexture();
	char rt_name[64];
	sprintf(rt_name, "Flares_%02d", g_counter++);
	bool is_ok = rt_output->createRT(rt_name, xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	assert(is_ok);

    rt_output2 = new CRenderToTexture();
    is_ok = rt_output2->createRT("Flares_RTarget", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
    assert(is_ok);

	tech = Resources.get("postfx_flare.tech")->as<CRenderTechnique>();
	mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
    distance_factors = VEC4(1, 1, 1, 1);

    static int g_blur_counter = 0;
    for (int i = 0; i < 6; ++i) {
        CBlurStep* s = new CBlurStep;

        char blur_name[64];
        sprintf(blur_name, "%s_%02d", "Flare", g_blur_counter);
        g_blur_counter++;

        is_ok &= s->create(blur_name, xres, yres);
        assert(is_ok);
        steps.push_back(s);
        xres /= 2;
        yres /= 2;
    }
}

CTexture* TCompRenderFlares::apply(CTexture* in_color, CTexture * in_lights) {

	if (!enabled)
		return in_color;

	CTraceScoped scope("TCompFlares");
    CRenderToTexture* rt = CRenderToTexture::getCurrentRT();

    CTexture* output = in_lights;
    for (auto s : steps) {

        output = s->apply(in_lights, global_distance, distance_factors, weights);
        in_lights = output;
    }

    rt->activateRT();

    in_color->activate(TS_ALBEDO);
    in_lights->activate(TS_EMISSIVE);

	tech->activate();
	mesh->activateAndRender();

	return rt;
}
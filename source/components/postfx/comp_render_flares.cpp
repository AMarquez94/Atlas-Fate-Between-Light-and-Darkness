#include "mcv_platform.h"
#include "comp_render_flares.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"
#include "render/blur_step.h"

DECL_OBJ_MANAGER("render_flares", TCompRenderFlares);

CRenderToTexture* TCompRenderFlares::rt_output = nullptr;
std::vector< CBlurStep* > TCompRenderFlares::t_steps;

// ---------------------
void TCompRenderFlares::debugInMenu() {
    ImGui::Checkbox("Enabled", &enabled);
}

void TCompRenderFlares::load(const json& j, TEntityParseContext& ctx) {

    TCompRenderBlur::load(j, ctx, false);
    enabled = j.value("enabled", enabled);
    global_distance = j.value("global_distance", 1.0f);
    distance_factors = VEC4(1, 1, 1, 1);

    weights.x = j.value("w0", 1.f);
    weights.y = j.value("w1", 1.f);
    weights.z = j.value("w2", 1.f);
    weights.w = j.value("w3", 1.f);

    if (j.value("box_filter", false))
        weights = VEC4(1, 1, 1, 1);
    else if (j.value("gauss_filter", false))
        weights = VEC4(70, 56, 28, 8);

    int nsteps = j.value("max_steps", 6);
    int xres = Render.width;
    int yres = Render.height;

    if (!rt_output) {
        static int g_counter = 0;
        rt_output = new CRenderToTexture();
        char rt_name[64];
        sprintf(rt_name, "Flares_%02d", g_counter++);
        bool is_ok = rt_output->createRT(rt_name, xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
        assert(is_ok);

        static int g_blur_counter = 0;
        for (int i = 0; i < 6; ++i) {
            CBlurStep* s = new CBlurStep;

            char blur_name[64];
            sprintf(blur_name, "%s_%02d", "Flare", g_blur_counter);
            g_blur_counter++;

            is_ok &= s->create(blur_name, xres, yres);
            assert(is_ok);
            t_steps.push_back(s);
            xres /= 2;
            yres /= 2;
        }
    }

	tech = Resources.get("postfx_flare.tech")->as<CRenderTechnique>();
	mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>(); 
    nactive_steps = (int)j.value("active_steps", t_steps.size());
}

CTexture* TCompRenderFlares::apply(CTexture* in_color, CTexture * in_lights) {

	if (!enabled)
		return in_color;

	CTraceScoped scope("TCompFlares");
    CRenderToTexture* rt = CRenderToTexture::getCurrentRT();

    CTexture* output = in_lights;
    for (auto s : t_steps) {

        output = s->apply(in_lights, global_distance, distance_factors, weights);
        in_lights = output;
    }

    rt->activateRT();

    in_lights->activate(TS_EMISSIVE);

	tech->activate();
	mesh->activateAndRender();

	return rt;
}
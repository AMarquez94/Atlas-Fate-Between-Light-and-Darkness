#include "mcv_platform.h"
#include "comp_render_bloom.h"
#include "render/texture/render_to_texture.h"
#include "resources/resources_manager.h"
#include "render/render_utils.h"
#include "render/blur_step.h"

DECL_OBJ_MANAGER("render_bloom", TCompRenderBloom);
CRenderToTexture* TCompRenderBloom::rt_highlights = nullptr;

// ---------------------
TCompRenderBloom::TCompRenderBloom()
	: cte_bloom("bloom")
{
	bool is_ok = cte_bloom.create(CB_BLOOM);
	assert(is_ok);
	// How we mix each downsampled scale
	add_weights = VEC4(0.360f, 0.520f, 0.300f, 0.720f);
}

TCompRenderBloom::~TCompRenderBloom() {
	cte_bloom.destroy();
}

void TCompRenderBloom::debugInMenu() {
	TCompRenderBlur::debugInMenu();
	ImGui::DragFloat("Threshold Min", &threshold_min, 0.01f, 0.f, 1.f);
	ImGui::DragFloat("Threshold Max", &threshold_max, 0.01f, 0.f, 2.f);
	ImGui::DragFloat("Multiplier", &multiplier, 0.01f, 0.f, 3.f);
	ImGui::DragFloat4("Add Weights", &add_weights.x, 0.02f, 0.f, 3.f);
}

void TCompRenderBloom::load(const json& j, TEntityParseContext& ctx) {

	if (j.count("weights"))
        add_weights = loadVEC4(j["weights"]);

	threshold_min = j.value("threshold_min", threshold_min);
	threshold_max = j.value("threshold_max", threshold_max);
    global_distance = j.value("global_distance", 1.f);
	multiplier = j.value("multiplier", 1.f);
    weights = VEC4(70, 56, 28, 8);

    enabled = j.value("enabled", true);
    global_distance = j.value("global_distance", 1.0f);
    distance_factors = VEC4(1, 2, 3, 4);

    if (j.value("box_filter", false))
        weights = VEC4(1, 1, 1, 1);
    else if (j.value("gauss_filter", false))
        weights = VEC4(70, 56, 28, 8);

    bool is_ok = true;
    int nsteps = j.value("max_steps", 2);
    int xres = Render.width;
    int yres = Render.height;

    if (!rt_highlights) {

        rt_highlights = new CRenderToTexture();
        char rt_name[64];
        sprintf(rt_name, "BloomFiltered_%08x", CHandle(this).asUnsigned());
        bool is_ok = rt_highlights->createRT(rt_name, Render.width, Render.height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
        assert(is_ok);

        static int g_blur_counter = 0;
        for (int i = 0; i < nsteps; ++i) {
            CBlurStep* s = new CBlurStep;

            char blur_name[64];
            sprintf(blur_name, "%s_%02d", "Bloom", g_blur_counter);
            g_blur_counter++;

            is_ok &= s->create(blur_name, xres, yres);
            assert(is_ok);
            steps.push_back(s);
            xres /= 2;
            yres /= 2;
        }
    }

	tech_filter = Resources.get("bloom_filter.tech")->as<CRenderTechnique>();
	tech_add = Resources.get("bloom_add.tech")->as<CRenderTechnique>();
	mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
}


void TCompRenderBloom::addBloom() {

	if (!enabled || nactive_steps == 0)
		return;

	cte_bloom.bloom_weights = add_weights * multiplier;
	cte_bloom.bloom_threshold_min = threshold_min * threshold_max;    // So min is always below max
	cte_bloom.bloom_threshold_max = threshold_max;
	cte_bloom.updateGPU();
	cte_bloom.activate();

	assert(mesh);
	assert(tech_add);

	// Activate the slots in the range 0..N
	// The slot0 gets the most blurred whites
	int i = nactive_steps - 1;
	int nslot = 0;
	while (nslot < 4 && i >= 0) {
		steps[i]->rt_output->activate(nslot);
		++nslot;
		--i;
	}

	tech_add->activate();
	mesh->activateAndRender();
}

// ---------------------------------------
void TCompRenderBloom::generateHighlights(CTexture* in_texture) {

	if (!enabled)
		return;

	// Filter input image to gather only the highlights
	auto prev_rt = rt_highlights->activateRT();
	assert(prev_rt);

	in_texture->activate(TS_ALBEDO);
	tech_filter->activate();
	mesh->activateAndRender();

	// Blur the highlights
	apply(rt_highlights);

	// Restore the prev rt
	prev_rt->activateRT();
}


#include "mcv_platform.h"
#include "comp_render_bloom.h"
#include "resources/resources_manager.h"
#include "render/blur_step.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("render_bloom", TCompRenderBloom);

// ---------------------
void TCompRenderBloom::debugInMenu() {

	ImGui::DragFloat("global_emissive_distance", &emissive_distance, 0.01f, 0.1f, 8.0f);
	ImGui::InputFloat("Emissive Weights Center", &emissive_weights.x);
	ImGui::InputFloat("Emissive Weights 1st", &emissive_weights.y);
	ImGui::InputFloat("Emissive Weights 2nd", &emissive_weights.z);
	ImGui::InputFloat("Emissive Weights 3rd", &emissive_weights.w);
	ImGui::InputFloat("Emissive Distance 2nd Tap", &emissive_factors.x);
	ImGui::InputFloat("Emissive Distance 3rd Tap", &emissive_factors.y);
	ImGui::InputFloat("Emissive Distance 4th Tap", &emissive_factors.z);

	TCompRenderBlur::debugInMenu();
}

void TCompRenderBloom::load(const json& j, TEntityParseContext& ctx) {

	TCompRenderBlur::load(j, ctx);
	
	xres = Render.width;
	yres = Render.height;
	static int g_bloom_counter = 0;
	rt_output = new CRenderToTexture();
	char rt_name[64];
	sprintf(rt_name, "Bloom_%02d", g_bloom_counter++);
	bool is_ok = rt_output->createRT(rt_name, xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	assert(is_ok);

	int nsteps = j.value("max_steps", 1);
	static int g_emissive_counter = 1;
	for (int i = 0; i < nsteps; ++i) {
		CBlurStep* s = new CBlurStep;

		char blur_name[64];
		sprintf(blur_name, "Emissive_Blur_%02d", g_emissive_counter);
		g_emissive_counter++;

		is_ok &= s->create(blur_name, xres, yres);
		assert(is_ok);
		emissive_steps.push_back(s);
		xres /= 2;
		yres /= 2;
	}

	emissive_weights = VEC4(4, 8, 16, 32);
	tech = Resources.get("bloom.tech")->as<CRenderTechnique>();
	mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
}

CTexture* TCompRenderBloom::apply( CTexture* in_texture, CTexture* in_light_texture, CTexture * emissive) {

  if (!enabled)
    return in_texture;

  CTraceScoped scope("CompBloom");

  CTexture* output = in_light_texture;
  int nsteps_to_apply = nactive_steps;
  for (auto s : steps) {
	  if (--nsteps_to_apply < 0)
		  break;
	  output = s->apply(in_light_texture, global_distance, distance_factors, weights);
	  in_light_texture = output;
  }

  CTexture* output2 = emissive;
  for (auto s : emissive_steps) {
	  output2 = s->apply(emissive, emissive_distance, emissive_factors, emissive_weights);
	  emissive = output2;
  }

  rt_output->activateRT();
  in_texture->activate(TS_ALBEDO);
  output->activate(TS_LIGHTMAP);
  output2->activate(TS_EMISSIVE);

  tech->activate();
  mesh->activateAndRender();

  return rt_output;
}
#include "mcv_platform.h"
#include "comp_render_bloom.h"
#include "resources/resources_manager.h"
#include "render/blur_step.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("render_bloom", TCompRenderBloom);

// ---------------------
void TCompRenderBloom::debugInMenu() {

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

	tech = Resources.get("bloom.tech")->as<CRenderTechnique>();
	mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
}

CTexture* TCompRenderBloom::apply( CTexture* in_texture, CTexture* in_light_texture) {

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

  rt_output->activateRT();
  in_texture->activate(TS_ALBEDO);
  output->activate(TS_EMISSIVE);

  tech->activate();
  mesh->activateAndRender();

  return rt_output;
}
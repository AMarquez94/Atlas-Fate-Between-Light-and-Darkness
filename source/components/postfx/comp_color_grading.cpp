#include "mcv_platform.h"
#include "comp_color_grading.h"
#include "render/texture/render_to_texture.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("color_grading", TCompColorGrading);

// ---------------------
void TCompColorGrading::debugInMenu() {
  ImGui::Checkbox("Enabled", &enabled);
  ImGui::DragFloat("Amount", &amount, 0.01f, 0.0f, 1.0f);
}

void TCompColorGrading::load(const json& j, TEntityParseContext& ctx) {
  enabled = j.value("enabled", true);
  amount= j.value( "amount", 1.0f);
  int xres = Render.width;
  int yres = Render.height;
  if (!rt) {
    rt = new CRenderToTexture;
    // Create a unique name for the rt 
    char rt_name[64];
    sprintf(rt_name, "ColorGrading_%08x", CHandle(this).asUnsigned() );
    bool is_ok = rt->createRT(rt_name, xres, yres, DXGI_FORMAT_B8G8R8A8_UNORM );
    assert(is_ok);
  }

  std::string lut_name = j.value("lut", "");
  lut1 = Resources.get(lut_name)->as<CTexture>();
  tech = Resources.get("color_grading.tech")->as<CRenderTechnique>();
  mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
}

CTexture* TCompColorGrading::apply( CTexture* in_texture) {
  if (!enabled)
    return in_texture;
  CTraceScoped scope("TCompColorGrading");

  // Upload to the GPU the how much visibe is the effect (defaults to 100%)
  cb_globals.global_shared_fx_amount = amount;
  cb_globals.updateGPU();

  rt->activateRT();
  lut1->activate(TS_LUT_COLOR_GRADING);
  in_texture->activate(TS_ALBEDO);
  tech->activate();
  mesh->activateAndRender();

  return rt;
}
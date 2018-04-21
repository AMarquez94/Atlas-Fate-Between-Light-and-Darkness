#include "mcv_platform.h"
#include "comp_render_blur_radial.h"
#include "resources/resources_manager.h"
#include "render/blur_step.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("render_blur_radial", TCompRenderBlurRadial);

// ---------------------
void TCompRenderBlurRadial::debugInMenu() {
  ImGui::Checkbox("Enabled", &enabled);
  ImGui::DragFloat2("Center", &normalized_center.x, 0.01f, -0.5f, 1.5f);
  ImGui::DragFloat("Amount", &amount, 0.01f, 0.0f, 5.0f);
  ImGui::DragFloat("Radius", &radius, 0.01f, 0.0f, 3.0f);
}

void TCompRenderBlurRadial::load(const json& j, TEntityParseContext& ctx) {
  enabled = j.value( "enabled", enabled);
  amount = j.value("amount", 1.0f);
  radius = j.value("radius", 1.0f);
  xres = Render.width;
  yres = Render.height;
  
  static int g_blur_radial_counter = 0;
  rt_output = new CRenderToTexture();
  char rt_name[64];
  sprintf(rt_name, "BlurRadial_%02d", g_blur_radial_counter++);
  bool is_ok = rt_output->createRT(rt_name, xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
  assert(is_ok);

  tech = Resources.get("blur_radial.tech")->as<CRenderTechnique>();
  mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();

  if( j.count( "center") )
    normalized_center = loadVEC2(j["center"]);

}

CTexture* TCompRenderBlurRadial::apply( CTexture* in_texture) {
  if (!enabled)
    return in_texture;
  CTraceScoped scope("CompBlur");

  rt_output->activateRT();
  in_texture->activate(TS_ALBEDO);

  cb_blur.blur_center = normalized_center;
  cb_blur.blur_d.x = amount;
  cb_blur.blur_d.y = radius;
  cb_blur.activate();
  cb_blur.updateGPU();

  tech->activate();
  mesh->activateAndRender();

  return rt_output;
}
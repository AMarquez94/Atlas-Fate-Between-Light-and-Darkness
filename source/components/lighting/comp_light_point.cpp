#include "mcv_platform.h"
#include "comp_light_point.h"
#include "../comp_transform.h"
#include "ctes.h"                     // texture slots
#include "render/render_manager.h" 
#include "render/render_objects.h" 

DECL_OBJ_MANAGER("light_point", TCompLightPoint);

DXGI_FORMAT readFormat(const json& j, const std::string& label);

// -------------------------------------------------
void TCompLightPoint::debugInMenu() {
  ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.f, 10.f);
  ImGui::ColorEdit3("Color", &color.x);
  ImGui::DragFloat("Radius", &radius, 0.01f, 0.f, 100.f);
}

MAT44 TCompLightPoint::getWorld() {
  TCompTransform* c = get<TCompTransform>();
  if (!c)
	  return MAT44::Identity;

  return MAT44::CreateScale(radius) * c->asMatrix();
}

// -------------------------------------------------
void TCompLightPoint::renderDebug() {
  // Render a wire sphere
  auto mesh = Resources.get("data/meshes/UnitSphere.mesh")->as<CRenderMesh>();
  renderMesh(mesh, getWorld(), VEC4(1, 1, 1, 1));
}

// -------------------------------------------------
void TCompLightPoint::load(const json& j, TEntityParseContext& ctx) {
  if(j.count("color"))
    color = loadVEC4(j["color"]);
  intensity = j.value("intensity", intensity);
  radius = j.value("radius", radius);
}

// -------------------------------------------------
// Updates the Shader Cte Light with MY information
void TCompLightPoint::activate() {
  TCompTransform* c = get<TCompTransform>();
  if (!c)
    return;

  cb_light.light_color = color;
  cb_light.light_intensity = intensity;
  cb_light.light_pos = c->getPosition();
  cb_light.light_radius = radius * c->getScale();
  cb_light.light_view_proj_offset = MAT44::Identity;
  cb_light.use_projector = 0;
  cb_light.updateGPU();
}




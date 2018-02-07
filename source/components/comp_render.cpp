#include "mcv_platform.h"
#include "comp_render.h"

DECL_OBJ_MANAGER("render", TCompRender);

#include "render/render_objects.h"
#include "render/texture/texture.h"
extern CVertexShader vs;
extern CPixelShader ps;
extern CVertexShader vs_obj;
extern CPixelShader ps_obj;

CRenderTechnique tech_solid = { &vs, &ps };
CRenderTechnique tech_objs = { &vs_obj, &ps_obj };

void TCompRender::debugInMenu() {
}

void TCompRender::load(const json& j, TEntityParseContext& ctx) {

  // Load some render mesh
  if (j.count("mesh")) {
    std::string name = j["mesh"];
    mesh = Resources.get(name)->as<CRenderMesh>();
    tech = &tech_objs;

    std::string texture_name = j["texture"];
    texture = Resources.get(texture_name)->as<CTexture>();
  }
  else {
    mesh = Resources.get("axis.mesh")->as<CRenderMesh>();
    tech = &tech_solid;
  }

}
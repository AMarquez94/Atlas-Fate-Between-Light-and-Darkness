#include "mcv_platform.h"
#include "render_utils.h"
#include "render_objects.h"

extern CRenderTechnique tech_solid;

void renderLine(VEC3 src, VEC3 dst, VEC4 color) {
  MAT44 world = MAT44::CreateLookAt(src, dst, VEC3(0, 1, 0)).Invert();
  float distance = VEC3::Distance(src, dst);
  world = MAT44::CreateScale(1, 1, -distance) * world;
  cb_object.obj_world = world;
  cb_object.obj_color = color;
  cb_object.updateGPU();

  tech_solid.activate();
  auto mesh = Resources.get("line.mesh")->as<CRenderMesh>();
  mesh->activateAndRender();
}



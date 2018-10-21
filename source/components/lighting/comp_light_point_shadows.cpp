#include "mcv_platform.h"
#include "comp_light_point_shadows.h"
#include "comp_transform.h"
#include "render/render_objects.h"    // cb_light
#include "render/texture/texture.h" 
#include "ctes.h"                     // texture slots
#include "render/texture/render_to_cube.h" 
#include "render/render_manager.h" 

DECL_OBJ_MANAGER("light_point_shadows", TCompLightPointShadows);

// We create a render target cube map, only having the ZBuffer, no color target
// It has 6 faces. The depth cube render target has also 6 depth stencil views
// each accessing one face of the cube
// When requested, we render ALL shadow casters in each face. Rendering everything 
// 6 times
// When when the deferred wants to render the shadow point maps, a sphere is
// rendered, and in the ps we access the computeCubeShadowFactor fn in the common.fx

// We could use a GS to generate a triangle on each render target so the CPU would only
// send the mesh just once.

DXGI_FORMAT readFormat(const json& j, const std::string& label);

// -------------------------------------------------
void TCompLightPointShadows::debugInMenu() {
  ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.f, 10.f);
  ImGui::ColorEdit3("Color", &color.x);
  ImGui::DragFloat("Step Size", &shadows_step, 0.01f, 0.f, 5.f);
  ImGui::Checkbox("Enabled", &shadows_enabled);
  float z_near = camera.getZNear();
  float z_far = camera.getZFar();
  bool changed = false;
  changed |= ImGui::DragFloat("Z Near", &z_near, 0.001f, 0.01f, 5.0f);
  changed |= ImGui::DragFloat("Z Far", &z_far, 1.0f, 2.0f, 3000.0f);
  if (changed)
    camera.setPerspective(deg2rad(90.0f), z_near, z_far);
}

// -------------------------------------------------
void TCompLightPointShadows::renderDebug() {
  for (int i = 0; i < shadows_cube_rt->getNSides(); ++i) {
    shadows_cube_rt->getCamera(i, &camera);
    renderCameraVolume(camera);
  }
  auto mesh = Resources.get("data/meshes/UnitSphere.mesh")->as<CRenderMesh>();
  renderMesh(mesh, getWorld(), VEC4(1, 1, 1, 1));
}

// -------------------------------------------------
void TCompLightPointShadows::load(const json& j, TEntityParseContext& ctx) {
  if(j.count("color"))
    color = loadVEC4(j["color"]);
  intensity = j.value("intensity", intensity);

  float z_near = j.value("z_near", z_near);
  float z_far = j.value("z_far", z_far);
  camera.setPerspective(deg2rad(90.0f), z_near, z_far);

  // if we need to allocate a shadow map
  shadows_step = j.value("shadows_step", shadows_step);
  shadows_resolution = j.value("shadows_resolution", shadows_resolution);
  auto shadowmap_fmt = readFormat(j, "shadows_fmt");
  assert(shadows_resolution > 0);
  shadows_cube_rt = new CRenderToCube;
  // Make a unique name to have the Resource Manager happy with the unique names for each resource
  char my_name[64];
  sprintf(my_name, "shadow_cubemap_%08x", CHandle(this).asUnsigned());
    
  // Added a placeholder Color Render Target to be able to do a alpha test when rendering
  // the grass
  bool is_ok = shadows_cube_rt->create(my_name, shadows_resolution, DXGI_FORMAT_UNKNOWN, shadowmap_fmt);
  assert(is_ok);

  shadows_enabled = j.value("enabled", shadows_enabled);
}

// -------------------------------------------------
// Updates the Shader Cte Light with MY information
void TCompLightPointShadows::activate() {
  TCompTransform* c = get<TCompTransform>();
  if (!c)
    return;

  cb_light.light_color = color;
  cb_light.light_intensity = intensity;
  cb_light.light_pos = c->getPosition();
  cb_light.light_radius = camera.getZFar();
  cb_light.updateGPU();

  cb_light.light_shadows_inverse_resolution = 1.0f / (float)shadows_cube_rt->getWidth();
  cb_light.light_shadows_step = shadows_step;
  cb_light.light_shadows_step_with_inv_res = shadows_step / (float)shadows_cube_rt->getWidth();
  cb_light.light_radius = 1.f;

  shadows_cube_rt->activateCubeShadowMap(TS_LIGHT_SHADOW_MAP);
}

MAT44 TCompLightPointShadows::getWorld() {
  TCompTransform* c = get<TCompTransform>();
  if (!c)
    return MAT44::Identity;
  return MAT44::CreateScale(camera.getZFar()) * c->asMatrix();
}


// ------------------------------------------------------
void TCompLightPointShadows::render() {
  activate();
  setWorldTransform(getWorld());
  auto* mesh = Resources.get("data/meshes/UnitSphere.mesh")->as<CRenderMesh>();
  mesh->activateAndRender();
}

// ------------------------------------------------------
void TCompLightPointShadows::generateShadowMap() {
  if (!shadows_cube_rt || !shadows_enabled )
    return;
  PROFILE_FUNCTION("LightPointShadows.generateShadowMap");

  // In this slot is where we activate the render targets that we are going
  // to update now. You can't be active as texture and render target at the
  // same time
  CTexture::setNullTexture(TS_LIGHT_SHADOW_MAP);

  static char* face_names[6] = {
    "CubeFace-0",
    "CubeFace-1",
    "CubeFace-2",
    "CubeFace-3",
    "CubeFace-4",
    "CubeFace-5",
  };

  // We need the have the camera.position in sync with the entity transform
  TCompTransform* my_trans = get<TCompTransform>();
  camera.lookAt(my_trans->getPosition(), my_trans->getPosition() + VEC3(0, 0, 1));

  CTraceScoped gpu_scope(shadows_cube_rt->getName().c_str());
  for (int i = 0; i < shadows_cube_rt->getNSides(); ++i) {
    CTraceScoped scope(face_names[i]);
    shadows_cube_rt->activateFace(i, &camera);
    shadows_cube_rt->clearDepthBuffer(i);
    
    // We are going to render the scene from the light position & orientation
    activateCamera(camera, shadows_cube_rt->getWidth(), shadows_cube_rt->getHeight());
    CRenderManager::get().renderCategory("shadows");
  }

}





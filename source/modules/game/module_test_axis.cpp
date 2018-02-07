#include "mcv_platform.h"
#include "module_test_axis.h"
#include "camera/camera.h"
#include "render/render_objects.h"
#include "entity/entity.h"
#include "modules/module_entities.h"
#include "resources/resources_manager.h"
#include "render/texture/texture.h"
#include "handle/handle.h"
#include "components/comp_name.h"
#include "components/comp_transform.h"
#include "components/comp_camera.h"
#include "entity/entity_parser.h"

extern CRenderTechnique tech_solid;

/*
struct TControllerFPS {
  
  VEC2    cursor;
  VEC3    speed;
  float   rotation_sensibility = deg2rad(65.f) / 150.0f;
  float   pan_sensibility = 0.7f;
  float   speed_reduction_factor = 0.95f;

  void update(CCamera& camera, float dt ) {
    
    VEC3 origin = camera.getPosition();

    speed *= speed_reduction_factor;

    float dyaw = 0.f, dpitch = 0.f;
    
    VEC3 front = camera.getFront();
    float yaw, pitch;
    getYawPitchFromVector(front, &yaw, &pitch);

    VEC2 prev_cursor = cursor;
    cursor = VEC2(ImGui::GetMousePos().x, ImGui::GetMousePos().y );
    VEC2 delta_cursor = cursor - prev_cursor;

    if (ImGui::IsMouseDown(1)) {
      if (delta_cursor.x || delta_cursor.y) {
        dyaw -= delta_cursor.x;
        dpitch -= delta_cursor.y;
      }

      if (isPressed('W'))
        speed.z += 1.f;
      if (isPressed('S'))
        speed.z -= 1.f;
      if (isPressed('A'))
        speed.x += 1.f;
      if (isPressed('D'))
        speed.x -= 1.f;
    }

    yaw = yaw + dyaw * rotation_sensibility;
    pitch = pitch + dpitch * rotation_sensibility;
    const float max_pitch = deg2rad(90.f - 1e-2f);
    if (pitch > max_pitch)
      pitch = max_pitch;
    else if (pitch < -max_pitch)
      pitch = -max_pitch;

    origin += camera.getLeft() * pan_sensibility * speed.x * dt;
    origin += camera.getFront() * pan_sensibility * speed.z * dt;
    origin += camera.getUp() * pan_sensibility * speed.y * dt;

    VEC3 new_front = getVectorFromYawPitch(yaw, pitch);
    VEC3 target = origin + new_front;
    camera.lookAt(origin, target, VEC3(0, 1, 0));
  }

  void debugInMenu() {
    ImGui::DragFloat("Rotation Sensibility", &rotation_sensibility, 0.001f, 0.001f, 0.01f);
    ImGui::DragFloat("Pan Sensibility", &pan_sensibility, 0.01f, 0.1f, 5.f);
    ImGui::DragFloat("Inertia", &speed_reduction_factor, 0.001f, 0.7f, 1.f);
  }

};
*/

CCamera        camera;
//TControllerFPS controller_fps;

bool CModuleTestAxis::start()
{
  // One time at boot
  Resources.registerResourceClass(getResourceClassOf<CTexture>());
  Resources.registerResourceClass(getResourceClassOf<CRenderMesh>());

  TEntityParseContext ctx;
  parseScene("data/scenes/scene_basic.json", ctx);
  
  camera.lookAt(VEC3(12.0f, 8.0f, 8.0f), VEC3::Zero, VEC3::UnitY);
  camera.setPerspective(60.0f * 180.f / (float)M_PI, 0.1f, 1000.f);

  // -------------------------------------------
  if (!cb_camera.create(CB_CAMERA))
    return false;
  // -------------------------------------------
  if (!cb_object.create(CB_OBJECT))
    return false;

  cb_object.activate();
  cb_camera.activate();

  return true;
}

bool CModuleTestAxis::stop()
{
  cb_camera.destroy();
  cb_object.destroy();
  return true;
}

void CModuleTestAxis::update(float delta)
{
  //controller_fps.update(camera, delta);
  //controller_fps.debugInMenu();
}

void CModuleTestAxis::render()
{
  tech_solid.activate();

  // Find the entity with name 'the_camera'
  CHandle h_e_camera = getEntityByName("the_camera");
  if (h_e_camera.isValid()) {
    CEntity* e_camera = h_e_camera;
    TCompCamera* c_camera = e_camera->get< TCompCamera >();
    assert(c_camera);
    activateCamera(*c_camera);
  }
  else {
    activateCamera(camera);
  }

  // Render the grid
  cb_object.obj_world = MAT44::Identity;
  cb_object.obj_color = VEC4(1,1,1,1);
  cb_object.updateGPU();

  auto grid = Resources.get("grid.mesh")->as<CRenderMesh>();
  grid->activateAndRender();
  auto axis = Resources.get("axis.mesh")->as<CRenderMesh>();
  axis->activateAndRender();

}
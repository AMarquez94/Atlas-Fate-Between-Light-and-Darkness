#pragma once

#include "../comp_base.h"
#include "../comp_camera.h"

class CTexture;
class CRenderToCube;

class TCompLightPointShadows : public TCompBase {

  // Light params
  VEC4            color = VEC4(1, 1, 1, 1);
  float           intensity = 1.0f;

  // Stores znear/zfar, and has fov = 90o
  CCamera         camera;

  // Shadows params
  int               shadows_resolution = 256;
  float             shadows_step = 1.f;
  CRenderToCube*    shadows_cube_rt = nullptr;
  bool              shadows_enabled = true; // To be updated in runtime

  void activate();
  MAT44 getWorld();

public:
  void debugInMenu();
  void renderDebug();
  void load(const json& j, TEntityParseContext& ctx);
  DECL_SIBLING_ACCESS();

  void generateShadowMap();
  void render();
};

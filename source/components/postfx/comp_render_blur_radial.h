#ifndef INC_COMPONENT_RENDER_BLUR_RADIAL_H_
#define INC_COMPONENT_RENDER_BLUR_RADIAL_H_

#include "components/comp_base.h"
#include <vector>

struct CBlurStep;
class  CTexture;
class CRenderToTexture;

// ------------------------------------
class TCompRenderBlurRadial : public TCompBase {
  CRenderToTexture*       rt_output = nullptr;
  const CRenderTechnique* tech = nullptr;
  const CRenderMesh*      mesh = nullptr;
  bool                    enabled = true;
  float                   amount = 1.f;
  float                   radius = 1.f;
  int                     xres = 0;
  int                     yres = 0;
  VEC2                    normalized_center = VEC2(0.5f, 0.5f);   // The range (0,0)<->(1,1) maps to screen

public:

  void  load(const json& j, TEntityParseContext& ctx);
  void  debugInMenu();
  CTexture* apply(CTexture* in_texture);
};

#endif

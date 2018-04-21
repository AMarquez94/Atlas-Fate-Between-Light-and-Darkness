#ifndef INC_COMPONENT_RENDER_BLUR_H_
#define INC_COMPONENT_RENDER_BLUR_H_

#include "components/comp_base.h"
#include <vector>

struct CBlurStep;
class  CTexture;

// ------------------------------------
struct TCompRenderBlur : public TCompBase {
  std::vector< CBlurStep* > steps;
  VEC4  weights;
  VEC4  distance_factors;     // 1 2 3 4
  float global_distance;
  int   nactive_steps;
  bool enabled;
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  CTexture* apply(CTexture* in_texture);

};

#endif

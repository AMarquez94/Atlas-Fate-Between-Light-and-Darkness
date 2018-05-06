#ifndef INC_COMPONENT_COLOR_GRADING_H_
#define INC_COMPONENT_COLOR_GRADING_H_

#include "components/comp_base.h"
#include "ctes.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompColorGrading : public TCompBase {
  bool                          enabled = true;
  float                         amount = 1.f;
  const CRenderTechnique*       tech = nullptr;
  const CRenderMesh*            mesh = nullptr;
  CRenderToTexture*             rt = nullptr;
  const CTexture*               lut1 = nullptr;

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  CTexture* apply(CTexture* in_texture);
};

#endif

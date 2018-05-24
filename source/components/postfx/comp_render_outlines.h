#ifndef INC_COMPONENT_RENDER_OUTLINES_H_
#define INC_COMPONENT_RENDER_OUTLINES_H_

#include "components/comp_base.h"
#include "ctes.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompRenderOutlines : public TCompBase {
  bool                          enabled = true;
  float                         amount = 1.f;
  const CRenderTechnique*       tech = nullptr;
  const CRenderMesh*            mesh = nullptr;

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void apply();
};

#endif

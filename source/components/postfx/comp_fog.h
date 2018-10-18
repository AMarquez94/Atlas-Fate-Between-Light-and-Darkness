#ifndef INC_COMPONENT_FOG_H_
#define INC_COMPONENT_FOG_H_

#include "components/comp_base.h"
#include "ctes.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompFog : public TCompBase {
  bool                          enabled = true;
  float                         amount = 1.f;
  const CRenderTechnique*       tech = nullptr;
  const CRenderMesh*            mesh = nullptr;
  CRenderToTexture*             rt = nullptr;

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  CTexture* apply(CTexture* in_texture, CTexture* in_texture_light);
};

#endif

#pragma once

#include "comp_base.h"

class CRenderMesh;
class CTexture;
class CRenderTechnique;

class TCompRender : public TCompBase {
public:

  const CRenderMesh* mesh = nullptr;
  const CTexture* texture = nullptr;
  CRenderTechnique* tech = nullptr;

  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);

  DECL_SIBLING_ACCESS();
};
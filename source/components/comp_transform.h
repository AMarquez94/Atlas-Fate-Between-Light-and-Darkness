#pragma once

#include "comp_base.h"
#include "geometry/transform.h"

class TCompTransform : public CTransform, public TCompBase {
public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
};
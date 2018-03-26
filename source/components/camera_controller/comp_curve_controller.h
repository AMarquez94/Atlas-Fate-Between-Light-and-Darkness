#pragma once

#include "components/comp_base.h"

class CCurve;

class TCompCurve : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  void setRatio(float ratio) { _ratio = ratio; }

private:
  VEC3 getTargetPos();

  const CCurve* _curve = nullptr;
  float _ratio = 0.f;
  float _speed = 0.f;
  CHandle _target;
  std::string _targetName;
  bool _automove = false;
};


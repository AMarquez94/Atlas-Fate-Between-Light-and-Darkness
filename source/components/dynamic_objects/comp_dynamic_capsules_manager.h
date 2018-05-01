#pragma once

#include "components/comp_base.h"

class TCompDynamicCapsulesManager: public TCompBase {

	DECL_SIBLING_ACCESS();

  VEC3 start_point;
  VEC3 end_point;
  float speed;
  int numberOfCapsules;

public:

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
};

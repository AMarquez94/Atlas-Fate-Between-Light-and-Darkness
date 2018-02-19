#pragma once

#include "../comp_base.h"
#include "geometry/transform.h"
#include "../ia/ai_controller.h"
#include "entity/common_msgs.h"

class TCompShadowController : public TCompBase {

	DECL_SIBLING_ACCESS();
public:

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void Init();
  
};
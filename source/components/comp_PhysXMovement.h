#pragma once

#include "comp_base.h"

class TCompPhysXMovement: public TCompBase {


public:
	

	VEC3 delta_movement = VEC3::Zero;
	DECL_SIBLING_ACCESS();

};
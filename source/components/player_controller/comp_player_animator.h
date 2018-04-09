#pragma once

#include "components/comp_base.h"
#include "components/skeleton/comp_skeleton.h"
#include "components/skeleton/comp_animator.h"

class TCompPlayerAnimator;

class TCompPlayerAnimator : public TCompAnimator
{
	DECL_SIBLING_ACCESS();
public:
	void debugInMenu();
	static void registerMsgs();
	void onCreated(const TMsgEntityCreated& msg);

	enum EAnimation { IDLE = 0, WALK, RUN, FALL, ATTACK_IDLE, ATTACK };
	void initializeAnimations();
	
};


#pragma once

#include "components/comp_base.h"
#include "components/skeleton/comp_skeleton.h"
#include "components/skeleton/comp_animator.h"

class TCompPatrolAnimator;

class TCompPatrolAnimator : public TCompAnimator
{
public:
	enum EAnimation { IDLE = 0, WALK, ATTACK, DEATH };
	struct TMsgExecuteAnimation {

		TCompPatrolAnimator::EAnimation animation;
		float speed;
		DECL_MSG_ID();
	};

	DECL_SIBLING_ACCESS();

	void debugInMenu();
	static void registerMsgs();
	void onCreated(const TMsgEntityCreated& msg);
	void playMsgAnimation(const TMsgExecuteAnimation& msg);

	
	void initializeAnimations();
	bool playAnimation(TCompPatrolAnimator::EAnimation animation, float speed = 1.0f);
};


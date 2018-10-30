#pragma once

#include "components/comp_base.h"
#include "components/skeleton/comp_skeleton.h"
#include "components/skeleton/comp_animator.h"

class TCompPatrolAnimator;

class TCompPatrolAnimator : public TCompAnimator
{
public:
	enum EAnimation { IDLE = 0, WALK, ATTACK, DEAD, RUN, SHOOT_INHIBITOR, REPAIR_PATROL, DIE, TURN_LEFT, TURN_RIGHT, SUSPECTING, PLAYER_FOUNDED, LOOKING_FOR_PLAYER, BEING_REPARED, WALK_FAST, CINEMATIC_DIE, CINEMATIC_DEAD, LOOKING_FOR };
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


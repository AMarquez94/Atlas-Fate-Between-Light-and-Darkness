#pragma once

#include "components/comp_base.h"
#include "components/skeleton/comp_skeleton.h"
#include "components/skeleton/comp_animator.h"

class TCompMimeticAnimator;

class TCompMimeticAnimator : public TCompAnimator
{
public:
	enum EAnimation { IDLE = 0, WALK, DEATH, RUN, DIE, TURN_LEFT, TURN_RIGHT, SUSPECTING, PLAYER_FOUNDED, LOOKING_FOR_PLAYER, ALARM, JUMP_TO_WALL, WALL_WALK, QUIT_WALL, IDLE_WALL, WAKE_UP, RETURN_TO_WALL, REST_IN_WALL};
	struct TMsgExecuteAnimation {

		TCompMimeticAnimator::EAnimation animation;
		float speed;
		DECL_MSG_ID();
	};

	DECL_SIBLING_ACCESS();

	void debugInMenu();
	static void registerMsgs();
	void onCreated(const TMsgEntityCreated& msg);
	void playMsgAnimation(const TMsgExecuteAnimation& msg);

	
	void initializeAnimations();
	bool playAnimation(TCompMimeticAnimator::EAnimation animation, float speed = 1.0f);
};


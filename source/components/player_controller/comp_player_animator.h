#pragma once

#include "components/comp_base.h"
#include "components/skeleton/comp_skeleton.h"
#include "components/skeleton/comp_animator.h"

class TCompPlayerAnimator;

class TCompPlayerAnimator : public TCompAnimator
{

private:
    bool isInNoClipMode = false;
public:
	enum EAnimation { IDLE = 0, WALK, WALK_SLOW, ATTACK, DEATH, DEAD, CROUCH_IDLE, CROUCH_WALK, CROUCH_WALK_SLOW, RUN, HIT_FRONT, HIT_BACK, FALL, LAND_SOFT, LAND_HARD, METRALLA_START, METRALLA_MIDDLE, METRALLA_FINISH, SM_ENTER, SM_EXIT, SM_POSE, GRAB_ENEMY, GRABING_ENEMY, OPEN_WEAPONS, SONDA_NORMAL, SONDA_CROUCH, HACK, PRESS_BUTTON, SM_ENTER_FALL, WALK_CINEMATIC, CINEMATIC_LISTEN_IDLE, CINEMATIC_LISTEN_CROUCH, ATTACK_MIMETIC,CINEMATIC_HARD_POSE, CINEMATIC_HARD_POSE_LOOKING, CINEMATIC_IDLE, CINEMATIC_KEYBOARD,CINEMATIC_LOOKCAPSULES, CINEMATIC_LOOKCAPSULES_POSE, CINEMATIC_MAINMENU_POSE};
	struct TMsgExecuteAnimation {

		TCompPlayerAnimator::EAnimation animation;
		float speed;
		DECL_MSG_ID();
	};
	
	DECL_SIBLING_ACCESS();

	void debugInMenu();
	static void registerMsgs();
	void onCreated(const TMsgEntityCreated& msg);
	void playMsgAnimation(const TMsgExecuteAnimation& msg);
	void onSceneStop(const TMsgScenePaused& msg);
	void onMsgNoClipToggle(const TMsgNoClipToggle& msg);
	
	void initializeAnimations();
	bool playAnimation(TCompPlayerAnimator::EAnimation animation, float speed = 1.0f);
};


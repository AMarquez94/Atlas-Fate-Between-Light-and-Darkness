#pragma once

#include "components/comp_base.h"

class TCompPlayerAnimator;

typedef void (TCompPlayerAnimator::*animationhandler)(float);

struct TMsgAnimation {
	animationhandler animation_state;
	DECL_MSG_ID();
};

class TCompPlayerAnimator : public TCompBase
{
	animationhandler state;

	
	

	DECL_SIBLING_ACCESS();

public:
	enum EAnimationType { IDLE = 0, WALK, WALK_SLOW, RUN, FALL, ATTACK_IDLE, ATTACK };
	static void registerMsgs();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

private:

	struct AnimationSet {

		EAnimationType animationType = EAnimationType::IDLE;
		int animationId1 = -1;
		std::string animationName = "";
		float weight = 1.0f;
	};

	void onAnimation(const TMsgAnimation& msg);

	std::string _animationName;
	float _time = 0.f;
};


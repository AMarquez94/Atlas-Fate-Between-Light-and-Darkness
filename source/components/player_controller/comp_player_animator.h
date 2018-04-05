#pragma once

#include "components/comp_base.h"
#include "components/skeleton/comp_skeleton.h"

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
	enum EAnimation { IDLE = 0, WALK, RUN, FALL, ATTACK_IDLE, ATTACK };
	enum EAnimationType { ACTION = 0, CYCLIC };
	enum EAnimationSize { SINGLE = 0, DOUBLE };
	static void registerMsgs();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	void initializeAnimations();

	bool playAnimation(EAnimation animation);

private:

	struct AnimationSet {

		EAnimation animation = EAnimation::IDLE;
		EAnimationType animationType = EAnimationType::ACTION;
		EAnimationSize animationSize = EAnimationSize::SINGLE;
		std::string animationName = "";
		std::string secondAnimationName = "";
		int animationId = -1;
		int secondAnimationId = -1;
		float weight = 1.0f;
	};

	std::map<EAnimation, AnimationSet> animationsMap;
	std::string _animationName;
	float _time = 0.f;

	void onAnimation(const TMsgAnimation& msg);
	void onCreated(const TMsgEntityCreated& msg);
	bool initializeAnimation(EAnimation animation, EAnimationType animationType, EAnimationSize animationSize, std::string animationName, std::string secondAnimationName, float weight);
};


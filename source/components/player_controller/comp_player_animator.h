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
	static void registerMsgs();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

private:
	void onAnimation(const TMsgAnimation& msg);

	std::string _animationName;
	float _time = 0.f;
};


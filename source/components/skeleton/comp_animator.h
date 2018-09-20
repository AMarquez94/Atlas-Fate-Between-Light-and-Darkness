#pragma once

#include "components/comp_base.h"
#include "components/skeleton/comp_skeleton.h"
#include "mcv_platform.h"

class TCompAnimator;

class TCompAnimator : public TCompBase
{
	DECL_SIBLING_ACCESS();

public:

	enum EAnimation { DEFAULT = 0 };
	enum EAnimationType { ACTION = 0, CYCLIC, CYCLIC_PARTIAL };
	enum EAnimationSize { SINGLE = 0, DOUBLE };

	struct AnimationSet {

		EAnimation animation = EAnimation::DEFAULT;
		EAnimationType animationType = EAnimationType::ACTION;
		EAnimationSize animationSize = EAnimationSize::SINGLE;
		std::string animationName = "";
		std::string secondAnimationName = "";
		int animationId = -1;
		int secondAnimationId = -1;
		float weight = 1.0f;
		float speed = 1.0f;
		bool rootMovement = false;
		bool rootRotationMovement = false;
	};

	CHandle ownHandle;
	AnimationSet actualCyclicAnimation;
	std::map<EAnimation, AnimationSet> animationsMap;
	std::map<std::string, EAnimation> stringToAnimationsMap;

	float _time = 0.f;

	static void registerMsgs();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	void initializeAnimations();

	bool initializeAnimation(
		EAnimation animation,
		EAnimationType animationType,
		EAnimationSize animationSize,
		std::string animationName,
		std::string secondAnimationName,
		float weight,
		float speed,
		bool rootMovement,
		bool rootRotationMovement = false);

	bool playAnimationConverted(EAnimation animation, float speed = 1.0f);
	bool isCyclic(EAnimation animation);
	bool isComposed(EAnimation animation);
	EAnimation actualAnimation();
	bool isPlayingAnimation(EAnimation animation);

	void removeAction(EAnimation animation);

	EAnimation getAnimationByName(std::string animation_name);
	float getAnimationDuration(EAnimation animation);

	void setFeetIds(std::vector<int> feetBonesId);
	void setFeetNumAndCalculate(int numFeet);
	std::vector<VEC3> getFeetPositions();
	
};

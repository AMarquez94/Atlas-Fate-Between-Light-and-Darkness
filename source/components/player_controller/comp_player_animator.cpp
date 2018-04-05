#include "mcv_platform.h"
#include "comp_player_animator.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("player_animator", TCompPlayerAnimator);

void TCompPlayerAnimator::debugInMenu() {
	//ImGui::DragFloat("Sensitivity", &_sensitivity, 0.01f, 0.001f, 0.1f);
	static float delta_movement = 0.0f;
	ImGui::Text("Animation name: %s", _animationName);

	if (ImGui::SmallButton("Idle")) {
		playAnimation(EAnimation::IDLE);
	}

	if (ImGui::SmallButton("Walk")) {
		playAnimation(EAnimation::WALK);
	}

	if (ImGui::SmallButton("Run")) {
		playAnimation(EAnimation::RUN);
	}

	if (ImGui::SmallButton("Attack")) {
		playAnimation(EAnimation::ATTACK);
	}

	ImGui::DragFloat("Delta Movement", &delta_movement, 0.01f, 0, 1.f);
	TCompSkeleton * compSkeleton = get<TCompSkeleton>();
	compSkeleton->setCyclicAnimationWeight(delta_movement);
}

void TCompPlayerAnimator::load(const json& j, TEntityParseContext& ctx) {
	//_sensitivity = j.value("sensitivity", _sensitivity);
	_animationName = "";	

}

void TCompPlayerAnimator::initializeAnimations() {
	
	initializeAnimation(
		EAnimation::IDLE,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"idle",
		"",
		1.0f
	);

	initializeAnimation(
		EAnimation::WALK,
		EAnimationType::CYCLIC,
		EAnimationSize::DOUBLE,
		"jog",
		"walk",
		0.0f
	);

	initializeAnimation(
		EAnimation::RUN,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"jog",
		"",
		1.0f
	);

	initializeAnimation(
		EAnimation::ATTACK,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"kick",
		"",
		1.0f
	);
}

bool TCompPlayerAnimator::initializeAnimation(EAnimation animation, EAnimationType animationType, EAnimationSize animationSize, std::string animationName, std::string secondAnimationName, float weight) {

	TCompSkeleton * compSkeleton = get<TCompSkeleton>();
	AnimationSet auxAnimSet;
	auxAnimSet.animation = animation;
	auxAnimSet.animationType = animationType;
	auxAnimSet.animationSize = animationSize;
	auxAnimSet.animationName = animationName;
	auxAnimSet.animationId = compSkeleton->getAnimationIdByName(auxAnimSet.animationName);
	if (auxAnimSet.animationId == -1) {
		fatal("The first animation non exists");
		return false;
	}
	auxAnimSet.secondAnimationName = secondAnimationName;
	auxAnimSet.secondAnimationId = compSkeleton->getAnimationIdByName(auxAnimSet.secondAnimationName);
	if (auxAnimSet.animationSize == EAnimationSize::DOUBLE && auxAnimSet.secondAnimationId == -1) {
		fatal("The second animation non exists");
		return false;
	}	
	auxAnimSet.weight = weight;
	animationsMap[animation] = auxAnimSet;
	return false;
}

void TCompPlayerAnimator::update(float dt)
{

}

void TCompPlayerAnimator::registerMsgs() {

	DECL_MSG(TCompPlayerAnimator, TMsgAnimation, onAnimation);
	DECL_MSG(TCompPlayerAnimator, TMsgEntityCreated, onCreated);
}

void TCompPlayerAnimator::onAnimation(const TMsgAnimation& msg)
{
	state = msg.animation_state;
	_time = 0.f;
}

void TCompPlayerAnimator::onCreated(const TMsgEntityCreated& msg) {

	TCompSkeleton * compSkeleton = get<TCompSkeleton>();
	assert(compSkeleton);
	initializeAnimations();
}

bool TCompPlayerAnimator::playAnimation(EAnimation animation) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		return false;
	}
	TCompSkeleton * compSkeleton = get<TCompSkeleton>();
	AnimationSet animSet = animationsMap[animation];

	switch (animSet.animationType)
	{
	case EAnimationType::CYCLIC:

		if (animSet.animationType == EAnimationSize::DOUBLE) {
			compSkeleton->changeCyclicAnimation(animSet.animationId, animSet.secondAnimationId, animSet.weight);
		}
		else {
			compSkeleton->changeCyclicAnimation(animSet.animationId);
		}
		break;
	
	case EAnimationType::ACTION :
		compSkeleton->executeActionAnimation(animSet.animationId);
		break;
	
	}

}

bool  TCompPlayerAnimator::isCyclic(EAnimation animation) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationSet animSet = animationsMap[animation];
	return animSet.animationType == EAnimationType::CYCLIC;
}

bool  TCompPlayerAnimator::isComposed(EAnimation animation) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationSet animSet = animationsMap[animation];
	return animSet.animationSize == EAnimationSize::DOUBLE;
}

TCompPlayerAnimator::EAnimation TCompPlayerAnimator::actualAnimation() {

	return actualCyclicAnimation.animation;
}

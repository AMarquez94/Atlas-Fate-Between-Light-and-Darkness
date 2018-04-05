#include "mcv_platform.h"
#include "comp_player_animator.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("player_animator", TCompPlayerAnimator);

void TCompPlayerAnimator::debugInMenu() {
	//ImGui::DragFloat("Sensitivity", &_sensitivity, 0.01f, 0.001f, 0.1f);
	ImGui::Text("Animation name: %s", _animationName);
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
		1.0f
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

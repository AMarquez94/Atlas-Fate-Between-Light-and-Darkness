#include "mcv_platform.h"
#include "comp_player_animator.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("player_animator", TCompPlayerAnimator);

void TCompPlayerAnimator::debugInMenu() {

	static float delta_movement = 0.0f;
	static float speed = 1.0f;
	ImGui::DragFloat("Speed", &speed, 0.01f, 0, 3.f);
	if (ImGui::SmallButton("Idle")) {
		playAnimation(EAnimation::IDLE, speed);
	}
	if (ImGui::SmallButton("Walk")) {
		playAnimation(EAnimation::WALK, speed);
	}
	if (ImGui::SmallButton("Attack")) {
		playAnimation(EAnimation::ATTACK, speed);
	}
	if (ImGui::SmallButton("Death")) {
		playAnimation(EAnimation::DEATH, speed);
	}
	if (ImGui::SmallButton("Crouch_walk")) {
		playAnimation(EAnimation::CROUCH_WALK, speed);
	}

	ImGui::DragFloat("Delta Movement", &delta_movement, 0.01f, 0, 1.f);
	TCompSkeleton * compSkeleton = get<TCompSkeleton>();
	compSkeleton->setCyclicAnimationWeight(delta_movement);
}

void TCompPlayerAnimator::initializeAnimations() {

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::IDLE,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"idle",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::WALK,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"walk",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::RUN,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"run",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::ATTACK,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"attack",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::DEATH,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"death",
		"",
		1.0f,
		1.0f
	);
	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::CROUCH_IDLE,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"crouch",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::FALL,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"fall",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::CROUCH_WALK,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"crouch_walk",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::CROUCH_WALK_SLOW,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"crouch_walk_slow",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::LAND_SOFT,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"land_soft",
		"",
		1.0f,
		1.0f
	);

}

void TCompPlayerAnimator::registerMsgs() {
	DECL_MSG(TCompPlayerAnimator, TMsgEntityCreated, onCreated);
	DECL_MSG(TCompPlayerAnimator, TMsgExecuteAnimation, playMsgAnimation);
}

void TCompPlayerAnimator::onCreated(const TMsgEntityCreated& msg) {

	ownHandle = CHandle(this).getOwner();
	CEntity *e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	assert(compSkeleton);
	initializeAnimations();
	setFeetNumAndCalculate(2);
}

bool TCompPlayerAnimator::playAnimation(TCompPlayerAnimator::EAnimation animation, float speed) {

	return playAnimationConverted((TCompAnimator::EAnimation)animation, speed);
}

void TCompPlayerAnimator::playMsgAnimation(const TMsgExecuteAnimation& msg) {

	playAnimation(msg.animation,msg.speed);
}
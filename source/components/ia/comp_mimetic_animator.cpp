#include "mcv_platform.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"
#include "components/ia/comp_mimetic_animator.h"

DECL_OBJ_MANAGER("mimetic_animator", TCompMimeticAnimator);

void TCompMimeticAnimator::debugInMenu() {

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

	ImGui::DragFloat("Delta Movement", &delta_movement, 0.01f, 0, 1.f);
	TCompSkeleton * compSkeleton = get<TCompSkeleton>();
	compSkeleton->setCyclicAnimationWeight(delta_movement);
}

void TCompMimeticAnimator::initializeAnimations() {

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
		"run",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::ATTACK,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"alert",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::DEATH,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"suspicious_start",
		"",
		1.0f,
		1.0f
	);
}

void TCompMimeticAnimator::registerMsgs() {
	DECL_MSG(TCompMimeticAnimator, TMsgEntityCreated, onCreated);
	DECL_MSG(TCompMimeticAnimator, TMsgExecuteAnimation, playMsgAnimation);
}

void TCompMimeticAnimator::onCreated(const TMsgEntityCreated& msg) {

	ownHandle = CHandle(this).getOwner();
	CEntity *e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	assert(compSkeleton);
	initializeAnimations();
	setFeetNumAndCalculate(2);
}

bool TCompMimeticAnimator::playAnimation(TCompMimeticAnimator::EAnimation animation, float speed) {

	return playAnimationConverted((TCompAnimator::EAnimation)animation, speed);
}

void TCompMimeticAnimator::playMsgAnimation(const TMsgExecuteAnimation& msg) {

	playAnimation(msg.animation,msg.speed);
}
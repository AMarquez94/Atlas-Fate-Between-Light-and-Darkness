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
		playAnimation((TCompAnimator::EAnimation)EAnimation::IDLE);
	}

	if (ImGui::SmallButton("Walk")) {
		playAnimation((TCompAnimator::EAnimation)EAnimation::WALK);
	}

	if (ImGui::SmallButton("Run")) {
		playAnimation((TCompAnimator::EAnimation)EAnimation::RUN);
	}

	if (ImGui::SmallButton("Attack")) {
		playAnimation((TCompAnimator::EAnimation)EAnimation::ATTACK);
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
		EAnimationSize::DOUBLE,
		"jog",
		"walk",
		0.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::RUN,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"jog",
		"",
		1.0f,
		1.0f
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::ATTACK,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"kick",
		"",
		1.0f,
		1.0f
	);
}

void TCompPlayerAnimator::registerMsgs() {
	DECL_MSG(TCompPlayerAnimator, TMsgEntityCreated, onCreated);
}

void TCompPlayerAnimator::onCreated(const TMsgEntityCreated& msg) {
	ownHandle = CHandle(this).getOwner();
	CEntity *e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	assert(compSkeleton);
	initializeAnimations();
}
#include "mcv_platform.h"
#include "comp_patrol_animator.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("patrol_animator", TCompPatrolAnimator);

void TCompPatrolAnimator::debugInMenu() {

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
		playAnimation(EAnimation::DEAD, speed);
	}
	if (ImGui::SmallButton("Die")) {
		playAnimation(EAnimation::DIE, speed);
	}
	if (ImGui::SmallButton("proves")) {
		playAnimation(EAnimation::TURN_RIGHT, speed);
	}
	//playAnimation(EAnimation::BEING_REPARED, speed);
	ImGui::DragFloat("Delta Movement", &delta_movement, 0.01f, 0, 1.f);
	TCompSkeleton * compSkeleton = get<TCompSkeleton>();
	compSkeleton->setCyclicAnimationWeight(delta_movement);
}

void TCompPatrolAnimator::initializeAnimations() {

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::IDLE,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"idle",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::WALK,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"walk",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::WALK_FAST,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"run",
		"",
		1.0f,
		0.55f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::SHOOT_INHIBITOR,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"walk",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::DIE ,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"die",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::DEAD,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"dead",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::ATTACK,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"attack",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::TURN_RIGHT,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"turn_right",
		"",
		1.0f,
		1.0f,
		false,
		true
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::TURN_LEFT,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"turn_right",
		"",
		1.0f,
		1.0f,
		false,
		true
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::RUN,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"run",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::SHOOT_INHIBITOR,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"inhibidor",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::BEING_REPARED,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"pruebas",
		"",
		1.0f,
		1.0f,
		false,
		true
	);

}

void TCompPatrolAnimator::registerMsgs() {
	DECL_MSG(TCompPatrolAnimator, TMsgEntityCreated, onCreated);
	DECL_MSG(TCompPatrolAnimator, TMsgExecuteAnimation, playMsgAnimation);
}

void TCompPatrolAnimator::onCreated(const TMsgEntityCreated& msg) {

	ownHandle = CHandle(this).getOwner();
	CEntity *e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	assert(compSkeleton);
	initializeAnimations();
	setFeetNumAndCalculate(2);
}

bool TCompPatrolAnimator::playAnimation(TCompPatrolAnimator::EAnimation animation, float speed) {

	return playAnimationConverted((TCompAnimator::EAnimation)animation, speed);
}

void TCompPatrolAnimator::playMsgAnimation(const TMsgExecuteAnimation& msg) {

	playAnimation(msg.animation,msg.speed);
}
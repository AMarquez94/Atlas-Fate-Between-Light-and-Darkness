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
	if (ImGui::SmallButton("Death")) {
		playAnimation(EAnimation::RETURN_TO_WALL, speed);
	}
	if (ImGui::SmallButton("Die")) {
		playAnimation(EAnimation::REST_IN_WALL, speed);
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
		1.0f,
		false

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
		(TCompAnimator::EAnimation)EAnimation::DEATH,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"death",
		"",
		1.0f,
		1.0f,
		true
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::DIE,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"death",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::ALARM,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"idle",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::JUMP_TO_WALL,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"wakeup_jump",
		"",
		1.0f,
		1.0f,
		true
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::LOOKING_FOR_PLAYER,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"idle",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::PLAYER_FOUNDED,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"idle",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::QUIT_WALL,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"idle",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::TURN_LEFT,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"rotate",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::TURN_RIGHT,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"rotate",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::SUSPECTING,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"idle",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::IDLE_WALL,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"pose_pared",
		"",
		1.0f,
		1.0f,
		false
	);
	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::WAKE_UP,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"wakeup",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::RETURN_TO_WALL,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"rest_jump",
		"",
		1.0f,
		1.0f,
		true
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::REST_IN_WALL,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"rest_wall",
		"",
		1.0f,
		1.0f,
		false
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
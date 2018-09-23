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
	if (ImGui::SmallButton("prova")) {
		playAnimation(EAnimation::HIT_BACK, speed);
	}
    if (ImGui::SmallButton("FALL")) {
        playAnimation(EAnimation::FALL, speed);
    }
    if (ImGui::SmallButton("SM_ENTER")) {
        playAnimation(EAnimation::SM_ENTER, speed);
    }
    if (ImGui::SmallButton("OPEN_WEAPONS")) {
        playAnimation(EAnimation::OPEN_WEAPONS, speed);
    }
	if (ImGui::SmallButton("stop SM")) {
		removeAction((TCompAnimator::EAnimation)EAnimation::SM_ENTER);
	}
	if (ImGui::SmallButton("sonda")) {
		playAnimation(EAnimation::SONDA_NORMAL, speed);
	}
	if (ImGui::SmallButton("sonda_crouch")) {
		playAnimation(EAnimation::SONDA_CROUCH, speed);
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

  /* TODO: Meter animación para andar despacio (ahora es la de andar pero a menos velocidad) */
  initializeAnimation(
    (TCompAnimator::EAnimation)EAnimation::WALK_SLOW,
    EAnimationType::CYCLIC,
    EAnimationSize::SINGLE,
    "walk",
    "",
    1.0f,
    0.5f,
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
		(TCompAnimator::EAnimation)EAnimation::DEATH,
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
		(TCompAnimator::EAnimation)EAnimation::CROUCH_IDLE,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"crouch",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::FALL,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"fall",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::CROUCH_WALK,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"crouch_walk",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::SONDA_NORMAL,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"sonda_normal",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::SONDA_CROUCH,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"sonda_crouch",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::CROUCH_WALK_SLOW,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"crouch_walk_slow",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::LAND_SOFT,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"land_soft",
		"",
		1.0f,
		1.0f,
		false
	);

	  initializeAnimation(
	    (TCompAnimator::EAnimation)EAnimation::LAND_HARD,
	    EAnimationType::ACTION,
	    EAnimationSize::SINGLE,
	    "land_hard",
	    "",
	    1.0f,
	    1.0f,
		false
	  );
	
	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::HIT_BACK,
		EAnimationType::CYCLIC,
		EAnimationSize::DOUBLE,
		"walk",
		"run",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::METRALLA_START,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"metralla_start",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::METRALLA_MIDDLE,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"metralla_middle",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::METRALLA_FINISH,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"metralla_finish",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::SM_ENTER,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"sm_enter",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::SM_POSE,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"sm_pose",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::GRAB_ENEMY,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"touch_enemy",
		"",
		1.0f,
		1.0f,
		false
	);
	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::GRABING_ENEMY,
		EAnimationType::CYCLIC,
		EAnimationSize::SINGLE,
		"touching_enemy",
		"",
		1.0f,
		1.0f,
		false
	);

	initializeAnimation(
		(TCompAnimator::EAnimation)EAnimation::OPEN_WEAPONS,
		EAnimationType::ACTION,
		EAnimationSize::SINGLE,
		"open_weapons",
		"",
		1.0f,
		1.0f,
		false
	);

}

void TCompPlayerAnimator::registerMsgs() {
	DECL_MSG(TCompPlayerAnimator, TMsgEntityCreated, onCreated);
	DECL_MSG(TCompPlayerAnimator, TMsgExecuteAnimation, playMsgAnimation);
	DECL_MSG(TCompPlayerAnimator, TMsgScenePaused, onSceneStop);
	DECL_MSG(TCompPlayerAnimator, TMsgNoClipToggle, onMsgNoClipToggle);
}

void TCompPlayerAnimator::onSceneStop(const TMsgScenePaused& msg) {
	//playAnimation(EAnimation::IDLE);
}

void TCompPlayerAnimator::onMsgNoClipToggle(const TMsgNoClipToggle & msg)
{
    isInNoClipMode = !isInNoClipMode;
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
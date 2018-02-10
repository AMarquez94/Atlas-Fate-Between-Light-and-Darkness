#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_player_controller.h"
#include "comp_transform.h"
#include "comp_render.h"
#include "entity/common_msgs.h"
#include "utils/utils.h"

DECL_OBJ_MANAGER("player_controller", TCompPlayerController);

void TCompPlayerController::debugInMenu() {
  ImGui::DragFloat("Speed", &walkSpeedFactor, 0.1f, 0.f, 20.f);
  ImGui::DragFloat("Rotation", &rotationSpeed, 0.1f, 0.f, 20.f);
  ImGui::Text("State: %s", stateName.c_str());
  ImGui::ProgressBar(stamina/maxStamina);
}

void TCompPlayerController::load(const json& j, TEntityParseContext& ctx) {
  walkSpeedFactor = j.value("speed", 1.0f);
  rotationSpeed = j.value("rotation_speed", 1.0f);

  Init();
}

void TCompPlayerController::Init() {
	AddState("idle", (statehandler)&TCompPlayerController::IdleState);
	AddState("motion", (statehandler)&TCompPlayerController::MotionState);
	AddState("smEnter", (statehandler)&TCompPlayerController::ShadowMergingEnterState);
	AddState("smHor", (statehandler)&TCompPlayerController::ShadowMergingHorizontalState);
	AddState("smVer", (statehandler)&TCompPlayerController::ShadowMergingVerticalState);
	AddState("smEnemy", (statehandler)&TCompPlayerController::ShadowMergingEnemyState);
	AddState("smLand", (statehandler)&TCompPlayerController::ShadowMergingLandingState);
	AddState("smExit", (statehandler)&TCompPlayerController::ShadowMergingExitState);
	AddState("attack", (statehandler)&TCompPlayerController::AttackState);
	AddState("rmInhibitor", (statehandler)&TCompPlayerController::RemovingInhibitorState);
	AddState("fall", (statehandler)&TCompPlayerController::FallingState);
	AddState("land", (statehandler)&TCompPlayerController::LandingState);
	AddState("hit", (statehandler)&TCompPlayerController::HitState);
	AddState("dead", (statehandler)&TCompPlayerController::DeadState);

	/* TODO: not for milestone1 */
	//AddState("probe", (statehandler)&TCompPlayerController::ProbeState);
	//AddState("push", (statehandler)&TCompPlayerController::PushState);

	ChangeState("idle");
}

void TCompPlayerController::registerMsgs() {

}


void TCompPlayerController::IdleState(float dt){
	stamina = Clamp<float>(stamina + (incrStamina * dt), minStamina, maxStamina);
	if (btShadowMerging.getsPressed() && checkShadows()) {
		ChangeState("smEnter");
	}
	else if (motionButtonsPressed()) {
		ChangeState("motion");
	}
	dbg("%f\n", stamina);
}



void TCompPlayerController::MotionState(float dt){ 
	stamina = Clamp<float>(stamina + (incrStamina * dt), minStamina, maxStamina);
	if (!motionButtonsPressed()) {
		ChangeState("idle");
	}
	else {
		
		movePlayer(dt);

		if (btShadowMerging.getsPressed() && checkShadows()) {
			ChangeState("smEnter");
		}
	}
	dbg("%f\n", stamina);
}


void TCompPlayerController::PushState(float dt){ 

}


void TCompPlayerController::AttackState(float dt){ 

}


void TCompPlayerController::ProbeState(float dt){ 

}


void TCompPlayerController::RemovingInhibitorState(float dt){ 

}


void TCompPlayerController::ShadowMergingEnterState(float dt){
	TCompRender* t = get<TCompRender>();
	t->color = VEC4(0, 0, 0, 0);
	ChangeState("smHor");
	dbg("%f\n", stamina);
}


void TCompPlayerController::ShadowMergingHorizontalState(float dt){ 

	if (motionButtonsPressed()) {
		movePlayer(dt);
		stamina = Clamp<float>(stamina - (dcrStaminaGround * dt), minStamina, maxStamina);
	}
	else {
		stamina = Clamp<float>(stamina - (dcrStaminaGround * dcrStaminaOnPlaceMultiplier * dt), minStamina, maxStamina);
	}
	
	if (!btShadowMerging.isPressed() || stamina == minStamina ) {
		ChangeState("smExit");
	}
	dbg("%f\n", stamina);

	
}


void TCompPlayerController::ShadowMergingVerticalState(float dt){ 

}


void TCompPlayerController::ShadowMergingEnemyState(float dt){

}


void TCompPlayerController::ShadowMergingLandingState(float dt){ 

}


void TCompPlayerController::ShadowMergingExitState(float dt){
	TCompRender* t = get<TCompRender>();
	t->color = VEC4(1, 1, 1, 1);
	ChangeState("idle");
	dbg("%f\n", stamina);
}


void TCompPlayerController::FallingState(float dt){

}


void TCompPlayerController::LandingState(float dt){

}


void TCompPlayerController::HitState(float dt){

}


void TCompPlayerController::DeadState(float dt){

}


bool TCompPlayerController::motionButtonsPressed() {
	return btUp.isPressed() || btDown.isPressed() || btLeft.isPressed() || btRight.isPressed();
}

bool TCompPlayerController::checkShadows() {

	/* TODO */
	return true && stamina > minStaminaToMerge;
}

void TCompPlayerController::movePlayer(float dt) {
	//Guardo mi transform
	TCompTransform *c_my_transform = get<TCompTransform>();

	//----------------------------------------------
	//Pongo a cero la velocidad actual
	float amount_moved = walkSpeedFactor * dt;

	//Detecto el teclado
	VEC3 local_speed = VEC3::Zero;
	if (btUp.isPressed())
		local_speed.z += 1.f;
	if (btDown.isPressed())
		local_speed.z -= 1.f;
	if (btLeft.isPressed())
		local_speed.x += 1.f;
	if (btRight.isPressed())
		local_speed.x -= 1.f;

	c_my_transform->setPosition(c_my_transform->getPosition() + local_speed * amount_moved);
}


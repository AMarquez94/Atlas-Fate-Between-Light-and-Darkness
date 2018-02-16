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
  ImGui::Text("Current speed: %f", currentSpeed);
  ImGui::Text("State: %s", stateName.c_str());
  if(inhibited)
	ImGui::Text("N times key pressed: %d", timesRemoveInhibitorKeyPressed);
  ImGui::ProgressBar(stamina/maxStamina);


  ImGui::Text("Buttons");

  ImGui::Text("%5s", "UP");
  ImGui::SameLine();
  ImGui::Text("%.2f", btUp.value);
  ImGui::SameLine();
  ImGui::ProgressBar(fabsf(btUp.value));

  ImGui::Text("%5s", "DOWN");
  ImGui::SameLine();
  ImGui::Text("%.2f", btDown.value);
  ImGui::SameLine();
  ImGui::ProgressBar(fabsf(btDown.value));
  
  ImGui::Text("%5s", "LEFT");
  ImGui::SameLine();
  ImGui::Text("%.2f", btLeft.value);
  ImGui::SameLine();
  ImGui::ProgressBar(fabsf(btLeft.value));
  
  ImGui::Text("%5s", "RIGHT");
  ImGui::SameLine();
  ImGui::Text("%.2f", btRight.value);
  ImGui::SameLine();
  ImGui::ProgressBar(fabsf(btRight.value));
}

void TCompPlayerController::load(const json& j, TEntityParseContext& ctx) {
	stamina = j.value("stamina", 100.f);
	maxStamina = j.value("maxStamina", 100.f);
	minStamina = j.value("minStamina", 0.f);
	minStaminaToMerge = j.value("minStaminaToMerge", 5.0f);
	dcrStaminaOnPlaceMultiplier = j.value("dcrStaminaOnPlaceMultiplier", 0.5f);
	dcrStaminaGround = j.value("dcrStaminaGround", 10.f);
	dcrStaminaWall = j.value("dcrStaminaWall", 20.f);
	incrStamina = j.value("incrStamina", 20.f);

	walkSpeedFactor = j.value("walkSpeedFactor", 3.0f);
	runSpeedFactor = j.value("runSpeedFactor", 6.0f);
	walkSlowSpeedFactor = j.value("walkSlowSpeedFactor", 1.5f);

	timesToPressRemoveInhibitorKey = j.value("timesToPressRemoveInhibitorKey", 10);

	currentSpeed = 0.f;
	target_name = j.value("target_camera", "");

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
	DECL_MSG(TCompPlayerController, TMsgPlayerHit, onMsgPlayerHit);
	DECL_MSG(TCompPlayerController, TMsgInhibitorShot, onMsgPlayerShotInhibitor);
}


void TCompPlayerController::IdleState(float dt){
	stamina = Clamp<float>(stamina + (incrStamina * dt), minStamina, maxStamina);
	if (inhibited) {
		manageInhibition(dt);
	}
	if (btShadowMerging.getsPressed() && checkShadows()) {
		timerForPressingRemoveInhibitorKey = 0.f;
		timesRemoveInhibitorKeyPressed = 0;
		ChangeState("smEnter");
	}
	else if (motionButtonsPressed()) {
		timerForPressingRemoveInhibitorKey = 0.f;
		timesRemoveInhibitorKeyPressed = 0;
		ChangeState("motion");
	}
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
}


void TCompPlayerController::FallingState(float dt){

}


void TCompPlayerController::LandingState(float dt){

}


void TCompPlayerController::HitState(float dt){

}


void TCompPlayerController::DeadState(float dt){

}


void TCompPlayerController::onMsgPlayerHit(const TMsgPlayerHit & msg)
{
	ChangeState("dead");

	TCompRender* t = get<TCompRender>();
	t->color = VEC4(1, 1, 1, 1);

	//TODO: Merge with Juan code
	//auto& handles = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
	//...
	//Notify all enemys that we are dead

	CEntity* enemy = (CEntity* )msg.h_sender;
	TMsgPlayerDead newMsg;
	newMsg.h_sender = CHandle(this).getOwner();
	enemy->sendMsg(newMsg);

	TCompTransform *mypos = get<TCompTransform>();
	float y, p, r;
	mypos->getYawPitchRoll(&y, &p, &r);
	p = p + deg2rad(90.f);
	mypos->setYawPitchRoll(y, p, r);
}

void TCompPlayerController::onMsgPlayerShotInhibitor(const TMsgInhibitorShot& msg) {
	if (!inhibited) {
		TCompRender * cRender = get<TCompRender>();
		cRender->color = VEC4(148, 0, 211, 1);
		inhibited = true;
	}
	else {
		timesRemoveInhibitorKeyPressed = 0;
	}
}

bool TCompPlayerController::motionButtonsPressed() {
	return btUp.isPressed() || btDown.isPressed() || btLeft.isPressed() || btRight.isPressed();
}

bool TCompPlayerController::checkShadows() {

	/* TODO */
	return true && stamina > minStaminaToMerge && !inhibited;
}

void TCompPlayerController::movePlayer(float dt) {

	// Player movement and rotation related method.
	float yaw, pitch, roll;
	float c_yaw, c_pitch, c_roll;
	CEntity *player_camera = (CEntity *)getEntityByName(target_name);

	TCompTransform *c_my_transform = get<TCompTransform>();
	TCompTransform * trans_camera = player_camera->get<TCompTransform>();
	trans_camera->getYawPitchRoll(&c_yaw, &c_pitch, &c_roll);
	c_my_transform->getYawPitchRoll(&yaw, &pitch, &roll);

	//----------------------------------------------
	//Pongo a cero la velocidad actual
	currentSpeed = 0;

	if (stateName.compare("smHor") == 0 || stateName.compare("smVer") == 0) {
		currentSpeed = walkSpeedFactor;
	}
	else {
		if (btRun.isPressed()) {
			currentSpeed = runSpeedFactor;
		}
		else if (btSlow.isPressed()) {
			currentSpeed = walkSlowSpeedFactor;
		}
		else if (btSlowAnalog.isPressed()) {
			currentSpeed = fabsf(btSlowAnalog.value) < deadzone ? walkSlowSpeedFactor : walkSpeedFactor;
		}
		else{
			currentSpeed = walkSpeedFactor;
		}
	}

	if (btUp.isPressed())
	{
		if (btUp.value == 1.0 || btUp.value > deadzone) {
			float diff = atan2(sin(c_yaw - yaw), cos(c_yaw - yaw));
			yaw = yaw + diff * rotationSpeed * dt;
			c_my_transform->setYawPitchRoll(yaw, pitch, roll);
		}

	}

	if (btDown.isPressed())
	{
		if (btDown.value == 1.0 || btDown.value < -deadzone) {
			float target_angle = c_yaw - deg2rad(180.f);
			float diff = atan2(sin(target_angle - yaw), cos(target_angle - yaw));
			yaw = yaw + diff * rotationSpeed * dt;
			c_my_transform->setYawPitchRoll(yaw, pitch, roll);
		}
	}

	if (btLeft.isPressed())
	{
		if (btLeft.value == 1.0 || btLeft.value < -deadzone) {
			float target_angle = c_yaw + deg2rad(90.f);
			float diff = atan2(sin(target_angle - yaw), cos(target_angle - yaw));
			yaw = yaw + diff * rotationSpeed * dt;
			c_my_transform->setYawPitchRoll(yaw, pitch, roll);
		}
	}

	if (btRight.isPressed())
	{
		if (btRight.value == 1.0 || btRight.value > deadzone) {
			float target_angle = c_yaw - deg2rad(90.f);
			float diff = atan2(sin(target_angle - yaw), cos(target_angle - yaw));
			yaw = yaw + diff * rotationSpeed * dt;
			c_my_transform->setYawPitchRoll(yaw, pitch, roll);
		}
	}

	float amount_moved = currentSpeed * dt;
	c_my_transform->setPosition(c_my_transform->getPosition() + c_my_transform->getFront() * amount_moved);
}

void TCompPlayerController::manageInhibition(float dt) {

	if (btSecAction.getsPressed()) {
 		timesRemoveInhibitorKeyPressed++;
		timerForPressingRemoveInhibitorKey = 1.f;
		if (timesRemoveInhibitorKeyPressed >= timesToPressRemoveInhibitorKey) {
			TCompRender* t = get<TCompRender>();
			t->color = VEC4(1, 1, 1, 1);
			inhibited = false;
		}
	}
	else {
		timerForPressingRemoveInhibitorKey -= dt;
		if (timerForPressingRemoveInhibitorKey < 0.f) {
			timerForPressingRemoveInhibitorKey = 0.f;
			timesRemoveInhibitorKeyPressed = 0;
		}
	}
}
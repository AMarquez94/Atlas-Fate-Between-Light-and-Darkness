#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_player_controller.h"
#include "comp_transform.h"
#include "comp_render.h"
#include "entity/common_msgs.h"
#include "utils/utils.h"
#include "render/mesh/mesh_loader.h"

DECL_OBJ_MANAGER("player_controller", TCompPlayerController);

void TCompPlayerController::debugInMenu() {
  ImGui::DragFloat("Speed", &walkSpeedFactor, 0.1f, 0.f, 20.f);
  ImGui::Text("Current speed: %f", currentSpeed);
  ImGui::Text("State: %s", stateName.c_str());
  if (auxStateName.compare("") != 0) {
	  ImGui::SameLine();
	  ImGui::Text(" - %s", auxStateName.c_str());
  }
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
	walkCrouchSpeedFactor = j.value("walkCrouchSpeedFactor", 2.0f);
	runSpeedFactor = j.value("runSpeedFactor", 6.0f);
	walkSlowSpeedFactor = j.value("walkSlowSpeedFactor", 1.5f);
	walkSlowCrouchSpeedFactor = j.value("walkSlowCrouchSpeedFactor", 0.5f);
	rotationSpeed = j.value("rotationSpeed", 5.0f);

	currentSpeed = 0.f;
	target_name = j.value("target_camera", "");

	// Manually loading the necessary meshes
	auto pj_idle = loadMesh("data/meshes/pj_idle.mesh");
	auto pj_attack = loadMesh("data/meshes/pj_attack.mesh");
	auto pj_fall = loadMesh("data/meshes/pj_fall.mesh");
	auto pj_walk = loadMesh("data/meshes/pj_walk.mesh");
	auto pj_run = loadMesh("data/meshes/pj_run.mesh");
	auto pj_crouch = loadMesh("data/meshes/pj_crouch.mesh");

	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_idle", (CRenderMesh*)pj_idle));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_attack", (CRenderMesh*)pj_attack));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_fall", (CRenderMesh*)pj_fall));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_walk", (CRenderMesh*)pj_walk));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_run", (CRenderMesh*)pj_run));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_crouch", (CRenderMesh*)pj_crouch));

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
	AddState("crouch", (statehandler)&TCompPlayerController::CrouchState);

	/* TODO: not for milestone1 */
	//AddState("probe", (statehandler)&TCompPlayerController::ProbeState);
	//AddState("push", (statehandler)&TCompPlayerController::PushState);

	ChangeState("idle");
}

void TCompPlayerController::registerMsgs() {

}


void TCompPlayerController::IdleState(float dt){

	TCompRender *c_my_render = get<TCompRender>();
	c_my_render->mesh = mesh_states.find("pj_idle")->second;
	stamina = Clamp<float>(stamina + (incrStamina * dt), minStamina, maxStamina);
	if (btShadowMerging.getsPressed() && checkShadows()) {
		ChangeState("smEnter");
	}
	else if (motionButtonsPressed()) {
		ChangeState("motion");
	}
	else if (btCrouch.isPressed()) {
		ChangeState("crouch");
	}
}



void TCompPlayerController::MotionState(float dt){ 

	stamina = Clamp<float>(stamina + (incrStamina * dt), minStamina, maxStamina);

	if (!motionButtonsPressed()) {
		auxStateName = "";
		ChangeState("idle");
	}
	else {
		
		movePlayer(dt);

		if (btShadowMerging.getsPressed() && checkShadows()) {
			auxStateName = "";
			ChangeState("smEnter");
		}
	}
}
void TCompPlayerController::CrouchState(float dt) {
	TCompRender *c_my_render = get<TCompRender>();
	c_my_render->mesh = mesh_states.find("pj_crouch")->second;
	if (btCrouch.getsReleased()) {
		ChangeState("idle");
	}
	if (btShadowMerging.getsPressed() && checkShadows()) {
		ChangeState("smEnter");
	}
	else if (motionButtonsPressed()) {
		ChangeState("motion");
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
	
	if (!btShadowMerging.isPressed() || stamina <= minStamina ) {
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


const bool TCompPlayerController::motionButtonsPressed() {
	return btUp.isPressed() || btDown.isPressed() || btLeft.isPressed() || btRight.isPressed();
}

bool TCompPlayerController::checkShadows() {

	/* TODO */
	return true && stamina > minStaminaToMerge;
}

void TCompPlayerController::movePlayer(const float dt) {

	// Player movement and rotation related method.
	float yaw, pitch, roll;
	float c_yaw, c_pitch, c_roll;
	CEntity *player_camera = (CEntity *)getEntityByName(target_name);

	TCompRender *c_my_render = get<TCompRender>();
	TCompTransform *c_my_transform = get<TCompTransform>();
	TCompTransform * trans_camera = player_camera->get<TCompTransform>();
	trans_camera->getYawPitchRoll(&c_yaw, &c_pitch, &c_roll);
	c_my_transform->getYawPitchRoll(&yaw, &pitch, &roll);

	//----------------------------------------------
	//Pongo a cero la velocidad actual

	currentSpeed = 0;

	if (stateName.compare("smHor") == 0 || stateName.compare("smVer") == 0) {
		//TODO: meter distintas velocidades para mayor control con teclado? (joystick da igual)
		currentSpeed = walkSpeedFactor;
	}
	else {
		if (btRun.isPressed()) {
			c_my_render->mesh = mesh_states.find("pj_run")->second;
			auxStateName = "running";
			currentSpeed = runSpeedFactor;
		}
		else if (btSlow.isPressed()) {
			if (btCrouch.isPressed()) {
				c_my_render->mesh = mesh_states.find("pj_crouch")->second;
				auxStateName = "crouch slow";
				currentSpeed = walkSlowCrouchSpeedFactor;
			}
			else {
				c_my_render->mesh = mesh_states.find("pj_walk")->second;
				auxStateName = "walking slow";
				currentSpeed = walkSlowSpeedFactor;
			}
		}
		else{
			if (btCrouch.isPressed()) {
				c_my_render->mesh = mesh_states.find("pj_crouch")->second;
				auxStateName = "crouch";
				currentSpeed = walkCrouchSpeedFactor;
			}
			else {
				c_my_render->mesh = mesh_states.find("pj_walk")->second;
				auxStateName = "walking";
				currentSpeed = walkSpeedFactor;
			}
		}
	}

	VEC3 dir = VEC3::Zero;
	float inputSpeed = Clamp(fabs(btHorizontal.value) + fabs(btVertical.value), 0.f, 1.f);
	float player_accel = inputSpeed * currentSpeed * dt;

	// Little hotfix to surpass negative values on analog pad

	if (btUp.isPressed() && btUp.value > 0) {
		dir += fabs(btUp.value) * getVectorFromYaw(c_yaw);
	}
	else if (btDown.isPressed()) {
		dir += fabs(btDown.value) * getVectorFromYaw(c_yaw - deg2rad(180.f));
	}
	if (btRight.isPressed() && btRight.value > 0) {
		dir += fabs(btRight.value) * getVectorFromYaw(c_yaw - deg2rad(90.f));
	}
	else if (btLeft.isPressed()) {
		dir += fabs(btLeft.value) * getVectorFromYaw(c_yaw + deg2rad(90.f));
	}
	dir.Normalize();

	float dir_yaw = getYawFromVector(dir);
	Quaternion my_rotation = c_my_transform->getRotation();
	Quaternion new_rotation = Quaternion::CreateFromYawPitchRoll(dir_yaw, pitch, 0);
	Quaternion quat = Quaternion::Lerp(my_rotation, new_rotation, rotationSpeed * dt);

	c_my_transform->setRotation(quat);
	c_my_transform->setPosition(c_my_transform->getPosition() + c_my_transform->getFront() * player_accel);
}
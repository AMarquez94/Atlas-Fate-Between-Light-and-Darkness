#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_player_controller.h"
#include "comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("player_controller", TCompPlayerController);

void TCompPlayerController::debugInMenu() {
  ImGui::DragFloat("Speed", &speedFactor, 0.1f, 0.f, 20.f);
  ImGui::DragFloat("Rotation", &rotationSpeed, 0.1f, 0.f, 20.f);
  ImGui::Text("State: %s", stateName.c_str());
}

void TCompPlayerController::load(const json& j, TEntityParseContext& ctx) {
  speedFactor = j.value("speed", 1.0f);
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
	AddState("push", (statehandler)&TCompPlayerController::PushState);
	AddState("attack", (statehandler)&TCompPlayerController::AttackState);
	AddState("rmInhibitor", (statehandler)&TCompPlayerController::RemovingInhibitorState);
	AddState("probe", (statehandler)&TCompPlayerController::ProbeState);
	AddState("fall", (statehandler)&TCompPlayerController::FallingState);
	AddState("land", (statehandler)&TCompPlayerController::LandingState);
	AddState("hit", (statehandler)&TCompPlayerController::HitState);
	AddState("dead", (statehandler)&TCompPlayerController::DeadState);

	ChangeState("idle");
}

void TCompPlayerController::update(float dt) {
  
  //Guardo mi transform
  TCompTransform *c_my_transform = get<TCompTransform>();

  //----------------------------------------------
  //Pongo a cero la velocidad actual
  float amount_moved = speedFactor * dt;
  float amount_rotated = rotationSpeed * dt;

  // Current orientation
  float current_yaw = 0.f;
  float current_pitch = 0.f;
  c_my_transform->getYawPitchRoll(&current_yaw, &current_pitch);

  //Detecto el teclado
  VEC3 local_speed = VEC3::Zero;
  if (isPressed('W'))
    local_speed.z += 1.f;
  if (isPressed('S'))
    local_speed.z -= 1.f;
  if (isPressed('A'))
    local_speed.x += 1.f;
  if (isPressed('D'))
    local_speed.x -= 1.f;
  if (isPressed('Q'))
    current_yaw += amount_rotated;
  if (isPressed('E'))
    current_yaw -= amount_rotated;

  //const Input::TInterface_Keyboard& kb = CEngine::get().getInput().host(Input::PLAYER_1).keyboard();
  const Input::TButton& bt = kb.key(VK_SPACE);
  if (bt.getsPressed()) {
    TEntityParseContext ctx;
    if (parseScene("data/prefabs/bullet.prefab", ctx)) {
      assert(!ctx.entities_loaded.empty());
      // Send the entity who has generated the bullet
      ctx.entities_loaded[0].sendMsg(TMsgAssignBulletOwner{ CHandle(this).getOwner() });
    }
  }

  // Using TransformNormal because I just want to rotate
  VEC3 world_speed = VEC3::TransformNormal(local_speed, c_my_transform->asMatrix());
  // Guardo la y de la posicion y le sumo la nueva posicion a la x y a la z
  VEC3 my_new_pos = c_my_transform->getPosition() + world_speed * amount_moved;
  c_my_transform->setYawPitchRoll(current_yaw, current_pitch, 0.f);

  //Actualizo la posicion del transform
  c_my_transform->setPosition(my_new_pos);
}

void TCompPlayerController::registerMsgs() {

}


void TCompPlayerController::IdleState(float dt){
	if(kb.key(0x57).getsPressed()){
		ChangeState("motion");
	}
}



void TCompPlayerController::MotionState(float dt){ 

	if (!kb.key(0x57).isPressed()) {

		ChangeState("idle");
	}
	else {
		VEC3 local_speed = VEC3::Zero;
		local_speed.z += 1.f;

		TCompTransform *c_my_transform = get<TCompTransform>();

		c_my_transform->setPosition(c_my_transform->getPosition() + local_speed * dt);
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

}


void TCompPlayerController::ShadowMergingHorizontalState(float dt){ 

}


void TCompPlayerController::ShadowMergingVerticalState(float dt){ 

}


void TCompPlayerController::ShadowMergingEnemyState(float dt){

}


void TCompPlayerController::ShadowMergingLandingState(float dt){ 

}


void TCompPlayerController::ShadowMergingExitState(float dt){

}


void TCompPlayerController::FallingState(float dt){

}


void TCompPlayerController::LandingState(float dt){

}


void TCompPlayerController::HitState(float dt){

}


void TCompPlayerController::DeadState(float dt){

}


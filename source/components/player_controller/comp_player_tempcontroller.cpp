#include "mcv_platform.h"
#include "comp_player_tempcontroller.h"
#include "components/comp_fsm.h"
#include "components/comp_tags.h"
#include "components/comp_render.h"
#include "components/ia/comp_bt_patrol.h"
#include "components/ia/comp_bt_mimetic.h"
#include "components/comp_transform.h"
#include "components/physics/comp_rigidbody.h"
#include "components/physics/comp_collider.h"
#include "components/player_controller/comp_shadow_controller.h"
#include "components/lighting/comp_emission_controller.h"
#include "physics/physics_collider.h"
#include "render/mesh/mesh_loader.h"
#include "components/comp_name.h"
#include "windows/app.h"
#include "components/comp_group.h"
#include "render/render_utils.h"

DECL_OBJ_MANAGER("player_tempcontroller", TCompTempPlayerController);

void TCompTempPlayerController::debugInMenu() {
}

void TCompTempPlayerController::renderDebug() {

	//UI Window's Size
	ImGui::SetNextWindowSize(ImVec2((float)CApp::get().xres, (float)CApp::get().yres), ImGuiCond_Always);
	//UI Window's Position
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	//Transparent background - ergo alpha = 0 (RGBA)
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	//Some style added
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 255.0f, 0.0f, 1.0f));

	ImGui::Begin("UI", NULL,
		ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);
	{
		ImGui::SetCursorPos(ImVec2(CApp::get().xres * 0.02f, CApp::get().yres * 0.06f));
		ImGui::Text("Stamina:");
		ImGui::SetCursorPos(ImVec2(CApp::get().xres * 0.05f + 25, CApp::get().yres * 0.05f));
		ImGui::ProgressBar(stamina / maxStamina, ImVec2(CApp::get().xres / 5.f, CApp::get().yres / 30.f));
		ImGui::Text("State: %s", dbCameraState.c_str());
    //ImGui::Text("VECTOR DIR: (%f - %f - %f)", debugDir.x, debugDir.y, debugDir.z);
    //ImGui::Text("VECTOR FRONT: (%f - %f - %f)", debugMyFront.x, debugMyFront.y, debugMyFront.z);

	}

	ImGui::End();
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(2);
}

void TCompTempPlayerController::load(const json& j, TEntityParseContext& ctx) {

	state = (actionhandler)&TCompTempPlayerController::idleState;

	auto pj_idle = loadMesh("data/meshes/pj_idle.mesh");
	auto pj_attack = loadMesh("data/meshes/pj_attack.mesh");
	auto pj_fall = loadMesh("data/meshes/pj_fall.mesh");
	auto pj_walk = loadMesh("data/meshes/pj_walk.mesh");
	auto pj_run = loadMesh("data/meshes/pj_run.mesh");
	auto pj_crouch = loadMesh("data/meshes/pj_crouch.mesh");
	auto pj_shadowmerge = Resources.get("axis.mesh")->as<CRenderMesh>();

	// Insert them in the map.
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_idle", (CRenderMesh*)pj_idle));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_attack", (CRenderMesh*)pj_attack));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_fall", (CRenderMesh*)pj_fall));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_walk", (CRenderMesh*)pj_walk));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_run", (CRenderMesh*)pj_run));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_crouch", (CRenderMesh*)pj_crouch));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_shadowmerge", (CRenderMesh*)pj_shadowmerge));

	playerColor.colorIdle = j.count("colorIdle") ? loadVEC4(j["colorIdle"]) : VEC4(1, 1, 1, 1);
	playerColor.colorDead = j.count("colorDead") ? loadVEC4(j["colorDead"]) : VEC4(0, 0, 0, 1);
	playerColor.colorInhib = j.count("colorInhib") ? loadVEC4(j["colorInhib"]) : VEC4(1, 0, 1, 1);
	playerColor.colorMerge = j.count("colorMerge") ? loadVEC4(j["colorMerge"]) : VEC4(0, 1, 1, 1);

	mergeAngle = j.value("mergeAngle", 0.45f);
	maxFallingTime = j.value("maxFallingTime", 0.8f);
	hardFallingTime = j.value("hardFallingTime", 0.5f);
	maxFallingDistance = j.value("maxFallingDistance", 1.5f);
	maxAttackDistance = j.value("maxAttackDistance", 1.f);
	minStamina = j.value("minStamina", 0.f);
	maxStamina = j.value("maxStamina", 100.f);
	incrStamina = j.value("incrStamina", 15.f);
	decrStaticStamina = j.value("decrStaticStamina", 0.75f),
	decrStaminaHorizontal = j.value("decrStaminaHorizontal", 12.5f);
	decrStaminaVertical = j.value("decrStaminaVertical", 17.5f);
	minStaminaChange = j.value("minStaminaChange", 15.f);
	auxCamera = j.value("auxCamera", "");
	timesRemoveInhibitorKeyPressed = j.value("timesRemoveInhibitorKeyPressed", -1);
	initialPoints = j.value("timesRemoveInhibitorKeyPressed", -1);
	paused = true;
}

/* Player controller main update */
void TCompTempPlayerController::update(float dt) {

	if (!paused && !isConsoleOn) {

		(this->*state)(dt);

		// Methods that always must be running on background
		isGrounded = groundTest(dt);
		isMerged = onMergeTest(dt);
		updateStamina(dt);
		updateShader(dt); // Move this to player render component...
		timeInhib += dt;
	}
}

void TCompTempPlayerController::registerMsgs() {

	DECL_MSG(TCompTempPlayerController, TMsgStateStart, onStateStart);
	DECL_MSG(TCompTempPlayerController, TMsgStateFinish, onStateFinish);
	DECL_MSG(TCompTempPlayerController, TMsgEntityCreated, onCreate);
	DECL_MSG(TCompTempPlayerController, TMsgPlayerHit, onPlayerHit);
	DECL_MSG(TCompTempPlayerController, TMsgPlayerDead, onPlayerKilled);
	DECL_MSG(TCompTempPlayerController, TMsgInhibitorShot, onPlayerInhibited);
	DECL_MSG(TCompTempPlayerController, TMsgPlayerIlluminated, onPlayerExposed);
	DECL_MSG(TCompTempPlayerController, TMsgScenePaused, onPlayerPaused);
	DECL_MSG(TCompTempPlayerController, TMsgConsoleOn, onConsoleChanged);
	DECL_MSG(TCompTempPlayerController, TMsgShadowChange, onShadowChange);
}

void TCompTempPlayerController::onShadowChange(const TMsgShadowChange& msg) {

	//VEC4 merged_color = msg.is_shadowed ? playerColor.colorMerge : playerColor.colorIdle;

	//TCompEmissionController * e_controller = get<TCompEmissionController>();
	//e_controller->blend(merged_color, .5);
}

void TCompTempPlayerController::onCreate(const TMsgEntityCreated& msg) {

	/* Variable initialization */
	TCompCollider * c_my_collider = get<TCompCollider>();

	pxShadowFilterData = new physx::PxFilterData();
	pxShadowFilterData->word0 = c_my_collider->config->group;
	pxShadowFilterData->word1 = FilterGroup::Wall;

	pxPlayerFilterData = new physx::PxFilterData();
	pxPlayerFilterData->word0 = c_my_collider->config->group;
	pxPlayerFilterData->word1 = FilterGroup::All;

	physx::PxFilterData pxFilterData;
	pxFilterData.word1 = FilterGroup::Scenario;
	PxPlayerDiscardQuery.data = pxFilterData;

	/* Initial reset messages */
	hitPoints = 0;
	stamina = 100.f;
	fallingTime = 0.f;
	currentSpeed = 4.f;
	initialPoints = 5;
	rotationSpeed = 10.f;
	fallingDistance = 0.f;
	isInhibited = isGrounded = isMerged = false;
	dbgDisableStamina = false;
	paused = false;
}

/* Call this function once the state has been changed */
void TCompTempPlayerController::onStateStart(const TMsgStateStart& msg) {

	if (msg.action_start != NULL) {

		state = msg.action_start;
		currentSpeed = msg.speed;

		TCompRigidbody * rigidbody = get<TCompRigidbody>();
		TCompTransform * t_trans = get<TCompTransform>();
		rigidbody->Resize(msg.size);

		physx::PxCapsuleController* caps = (physx::PxCapsuleController*)rigidbody->controller;
		if (caps != nullptr) {
			caps->setRadius(msg.radius);
			caps->setFootPosition(physx::PxExtendedVec3(t_trans->getPosition().x, t_trans->getPosition().y, t_trans->getPosition().z));
		}

		// Get the target camera and set it as our new camera.
		if (msg.target_camera) {
      CHandle new_camera = getEntityByName(msg.target_camera->name);
      if (new_camera != target_camera) {
			  Engine.getCameras().blendOutCamera(target_camera, msg.target_camera->blendOut);
      }
      target_camera = new_camera;
			Engine.getCameras().blendInCamera(target_camera, msg.target_camera->blendIn, CModuleCameras::EPriority::GAMEPLAY);
		}
		else {
			target_camera = getEntityByName("TPCamera"); //replace this
		}

		TMsgMakeNoise msgToSend;
		msgToSend.isOnlyOnce = msg.noise->isOnlyOnce;
		msgToSend.noiseRadius = msg.noise->noiseRadius;
		msgToSend.timeToRepeat = msg.noise->timeToRepeat;
		msgToSend.isArtificial = msg.noise->isArtificial;
		TCompGroup * tGroup = get<TCompGroup>();
		if (tGroup) {
			CEntity * eNoiseEmitter = tGroup->getHandleByName("Noise Emitter");
			eNoiseEmitter->sendMsg(msgToSend);
		}
	}
}

/* Call this function once the state has finished */
void TCompTempPlayerController::onStateFinish(const TMsgStateFinish& msg) {
	(this->*msg.action_finish)();
}

void TCompTempPlayerController::onPlayerHit(const TMsgPlayerHit & msg)
{
	CEntity* e = CHandle(this).getOwner();
	TMsgSetFSMVariable groundMsg;
	groundMsg.variant.setName("onDead");
	groundMsg.variant.setBool(true);
	e->sendMsg(groundMsg);
}

void TCompTempPlayerController::onPlayerKilled(const TMsgPlayerDead & msg)
{
	CEntity* e = CHandle(this).getOwner();
	TMsgSetFSMVariable groundMsg;
	groundMsg.variant.setName("onDead");
	groundMsg.variant.setBool(true);
	e->sendMsg(groundMsg);
}

void TCompTempPlayerController::onPlayerInhibited(const TMsgInhibitorShot & msg)
{
	if (!isInhibited) {
		isInhibited = true;

		//TCompEmissionController * e_controller = get<TCompEmissionController>();
		//e_controller->blend(playerColor.colorInhib, .1f);
	}
	timesRemoveInhibitorKeyPressed = initialPoints;

}

void TCompTempPlayerController::onPlayerExposed(const TMsgPlayerIlluminated & msg)
{
  if (isMerged && msg.isIlluminated) {
	  CEntity* e = CHandle(this).getOwner();
	  TMsgSetFSMVariable notMergeMsg;
	  notMergeMsg.variant.setName("onmerge");
	  notMergeMsg.variant.setBool(false); // & isGrounded
	  isMerged = false;
	  e->sendMsg(notMergeMsg);
  }
}

void TCompTempPlayerController::onPlayerPaused(const TMsgScenePaused& msg) {

	paused = msg.isPaused;
}

void TCompTempPlayerController::onConsoleChanged(const TMsgConsoleOn & msg)
{
	isConsoleOn = msg.isConsoleOn;
}

/* Idle state method, no logic yet */
void TCompTempPlayerController::idleState(float dt) {

}

/* Main thirdperson player motion movement handled here */
void TCompTempPlayerController::walkState(float dt) {

	// Player movement and rotation related method.
	float yaw, pitch, roll;
	CEntity *player_camera = target_camera;
	TCompTransform *c_my_transform = get<TCompTransform>();
	TCompTransform * trans_camera = player_camera->get<TCompTransform>();
	c_my_transform->getYawPitchRoll(&yaw, &pitch, &roll);

	float inputSpeed = Clamp(fabs(EngineInput["Horizontal"].value) + fabs(EngineInput["Vertical"].value), 0.f, 1.f);
	float player_accel = inputSpeed * currentSpeed * dt;

	VEC3 up = trans_camera->getFront();
	VEC3 normal_norm = c_my_transform->getUp();
	VEC3 proj = projectVector(up, normal_norm);
	VEC3 dir = getMotionDir(proj, normal_norm.Cross(-proj));


	if (dir == VEC3::Zero) dir = proj;

	float dir_yaw = getYawFromVector(dir);
	Quaternion my_rotation = c_my_transform->getRotation();
	Quaternion new_rotation = Quaternion::CreateFromYawPitchRoll(dir_yaw, pitch, 0);
	Quaternion quat = Quaternion::Lerp(my_rotation, new_rotation, rotationSpeed * dt);
	c_my_transform->setRotation(quat);
	c_my_transform->setPosition(c_my_transform->getPosition() + dir * player_accel);
}

/* Player motion movement when is shadow merged, tests included */
void TCompTempPlayerController::mergeState(float dt) {

	// Player movement and rotation related method.
	CEntity *player_camera = target_camera;
	TCompRigidbody * rigidbody = get<TCompRigidbody>();
	TCompTransform *c_my_transform = get<TCompTransform>();
	TCompTransform * trans_camera = player_camera->get<TCompTransform>();
	float angle_test = fabs(EnginePhysics.gravity.Dot(c_my_transform->getUp()));

	VEC3 prevUp = c_my_transform->getUp();
	VEC3 up = angle_test < mergeAngle ? -EnginePhysics.gravity : trans_camera->getFront();
	float inputSpeed = Clamp(fabs(EngineInput["Horizontal"].value) + fabs(EngineInput["Vertical"].value), 0.f, 1.f);
	float player_accel = inputSpeed * currentSpeed * dt;

	VEC3 normal_norm = rigidbody->normal_gravity;
	normal_norm.Normalize();
	VEC3 proj = projectVector(up, normal_norm);
	VEC3 dir = getMotionDir(proj, normal_norm.Cross(proj));

	if (dir == VEC3::Zero) dir = proj;

	if (tempInverseVerticalMovementMerged) dir.y = abs(dir.y) * -1;

	VEC3 new_pos = c_my_transform->getPosition() - dir;
	Matrix test = Matrix::CreateLookAt(c_my_transform->getPosition(), new_pos, c_my_transform->getUp()).Transpose();
	Quaternion quat = Quaternion::CreateFromRotationMatrix(test);
	c_my_transform->setRotation(quat);
	c_my_transform->setPosition(c_my_transform->getPosition() + dir * player_accel);

	if (convexTest() || concaveTest()) {

    VEC3 postUp = c_my_transform->getUp();

		angle_test = fabs(EnginePhysics.gravity.Dot(prevUp));
    float angle_amount = fabsf(acosf(prevUp.Dot(postUp)));
		std::string target_name = angle_test < mergeAngle ? "SMCameraVer" : "SMCameraHor";
		
    CEntity* e_target_camera = target_camera;
    if (angle_amount > deg2rad(30.f) || target_name.compare(dbCameraState) != 0) {

      /* Only "change" cameras when the amount of degrees turned is more than 30º */
      CEntity* eCamera = getEntityByName("SMCameraAux");
      TCompName * name = ((CEntity*)target_camera)->get<TCompName>();
      VEC3 dirToLookAt = -(prevUp + postUp);
      dirToLookAt.Normalize();

      TMsgSetCameraActive msg;
      msg.previousCamera = name->getName();
      target_camera = getEntityByName(target_name);
      msg.actualCamera = target_name;
      msg.directionToLookAt = dirToLookAt;
      eCamera->sendMsg(msg);
    }
    dbCameraState = target_name;
	}
  //debugMyFront = c_my_transform->getFront();
}

/* Resets the player to it's default state parameters */
void TCompTempPlayerController::resetState(float dt) {

	CEntity *player_camera = target_camera;
	TCompRigidbody *rigidbody = get<TCompRigidbody>();
	TCompTransform *c_my_transform = get<TCompTransform>();
	TCompTransform * trans_camera = player_camera->get<TCompTransform>();

	VEC3 up = trans_camera->getFront();
	VEC3 proj = projectVector(up, -EnginePhysics.gravity);
	VEC3 dir = getMotionDir(proj, EnginePhysics.gravity.Cross(proj));

	// Set collider gravity settings
	rigidbody->SetUpVector(-EnginePhysics.gravity);
	rigidbody->normal_gravity = EnginePhysics.gravityMod * EnginePhysics.gravity;

	if (dir == VEC3::Zero) dir = proj;

	VEC3 new_pos = c_my_transform->getPosition() - dir;
	Matrix test = Matrix::CreateLookAt(c_my_transform->getPosition(), new_pos, -EnginePhysics.gravity).Transpose();
	Quaternion quat = Quaternion::CreateFromRotationMatrix(test);
	c_my_transform->setRotation(quat);
}

void TCompTempPlayerController::exitMergeState(float dt)
{
	TMsgSetCameraCancelled msg;
	CEntity * eCamera = getEntityByName(auxCamera);
	eCamera->sendMsg(msg);

	CEntity *e = CHandle(this).getOwner();
	TMsgSetFSMVariable crouch;
	crouch.variant.setName("crouch");
	crouch.variant.setBool(false);
	e->sendMsg(crouch);
}

/* Player dead state */
void TCompTempPlayerController::deadState(float dt)
{
	TMsgPlayerDead newMsg;
	newMsg.h_sender = CHandle(this).getOwner();
	auto& handles = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
	for (auto h : handles) {
		CEntity* enemy = h;
		enemy->sendMsg(newMsg);
	}

	//TCompEmissionController * e_controller = get<TCompEmissionController>();
	//e_controller->blend(playerColor.colorDead, 3);

	state = (actionhandler)&TCompTempPlayerController::idleState;
}

void TCompTempPlayerController::removingInhibitorState(float dt) {

	CEntity* player = CHandle(this).getOwner();

	TMsgSetFSMVariable hitPoints;
	hitPoints.variant.setName("hitPoints");
	hitPoints.variant.setBool(false);
	player->sendMsg(hitPoints);

	TMsgSetFSMVariable finished;
	finished.variant.setName("inhibitor_removed");
	finished.variant.setBool(false);
	player->sendMsg(finished);

	TMsgSetFSMVariable inhibitor_try_to_remove;
	inhibitor_try_to_remove.variant.setName("inhibitor_try_to_remove");
	inhibitor_try_to_remove.variant.setBool(false);
	player->sendMsg(inhibitor_try_to_remove);

	if (timesRemoveInhibitorKeyPressed > 0) {

		timesRemoveInhibitorKeyPressed--;
		if (timesRemoveInhibitorKeyPressed == 0) {
			timesRemoveInhibitorKeyPressed = 0;
			isInhibited = false;

			TMsgSetFSMVariable finished;
			finished.variant.setName("inhibitor_removed");
			finished.variant.setBool(true);
			player->sendMsg(finished);
		}
		else {
			TMsgSetFSMVariable inhibitor_try_to_remove;
			inhibitor_try_to_remove.variant.setName("inhibitor_try_to_remove");
			inhibitor_try_to_remove.variant.setBool(true);
			player->sendMsg(inhibitor_try_to_remove);
		}
	}

}

/* Concave test, this determines if there is a surface normal change on concave angles */
const bool TCompTempPlayerController::concaveTest(void) {

	physx::PxRaycastHit hit;
	TCompRigidbody *rigidbody = get<TCompRigidbody>();
	TCompTransform *c_my_transform = get<TCompTransform>();
	VEC3 upwards_offset = c_my_transform->getPosition() + c_my_transform->getUp() * .01f;

	if (EnginePhysics.Raycast(upwards_offset, c_my_transform->getFront(), 0.35f + .1f, hit, physx::PxQueryFlag::eSTATIC, PxPlayerDiscardQuery))
	{
		VEC3 hit_normal = VEC3(hit.normal.x, hit.normal.y, hit.normal.z);
		VEC3 hit_point = VEC3(hit.position.x, hit.position.y, hit.position.z);
		if (hit_normal == c_my_transform->getUp()) return false;

		if (EnginePhysics.gravity.Dot(hit_normal) < .01f)
		{
			VEC3 new_forward = hit_normal.Cross(c_my_transform->getLeft());
			VEC3 target = c_my_transform->getPosition() + new_forward;

			rigidbody->SetUpVector(hit_normal);
			rigidbody->normal_gravity = EnginePhysics.gravityMod * -hit_normal;

			Matrix test = Matrix::CreateLookAt(c_my_transform->getPosition(), target, hit_normal).Transpose();
			Quaternion new_rotation = Quaternion::CreateFromRotationMatrix(test);
			VEC3 new_pos = hit_point;
			c_my_transform->setRotation(new_rotation);
			c_my_transform->setPosition(new_pos);
			return true;
		}
	}

	return false;
}

/* Convex test, this determines if there is a surface normal change on convex angles */
const bool TCompTempPlayerController::convexTest(void) {

	physx::PxRaycastHit hit;
	TCompTransform *c_my_transform = get<TCompTransform>();
	TCompRigidbody *rigidbody = get<TCompRigidbody>();
	VEC3 upwards_offset = c_my_transform->getPosition() + c_my_transform->getUp() * .01f;
	VEC3 new_dir = c_my_transform->getUp() + c_my_transform->getFront();
	new_dir.Normalize();

	if (EnginePhysics.Raycast(upwards_offset, -new_dir, 0.95f, hit, physx::PxQueryFlag::eSTATIC, PxPlayerDiscardQuery))
	{
		VEC3 hit_normal = VEC3(hit.normal.x, hit.normal.y, hit.normal.z);
		VEC3 hit_point = VEC3(hit.position.x, hit.position.y, hit.position.z);
		if (hit_normal == c_my_transform->getUp()) return false;

		if (hit.distance > .015f && EnginePhysics.gravity.Dot(hit_normal) < .01f)
		{
			VEC3 new_forward = -hit_normal.Cross(c_my_transform->getLeft());
			VEC3 target = hit_point + new_forward;

			if (hit_normal.y < c_my_transform->getUp().y  && EngineInput["btUp"].value > 0) tempInverseVerticalMovementMerged = true;

			rigidbody->SetUpVector(hit_normal);
			rigidbody->normal_gravity = EnginePhysics.gravityMod * -hit_normal;

			QUAT new_rotation = createLookAt(hit_point, target, hit_normal);
			VEC3 new_pos = hit_point + 0.3f * new_forward;
			c_my_transform->setRotation(new_rotation);
			c_my_transform->setPosition(new_pos);
			return true;
		}
	}

	return false;
}

/* Bitshifting test to determine if we are merged within the shadows */
const bool TCompTempPlayerController::onMergeTest(float dt) {

	CEntity* e = CHandle(this).getOwner();
	TCompRigidbody *c_my_rigidbody = get<TCompRigidbody>();
	TCompShadowController * shadow_oracle = get<TCompShadowController>();

	// Tests: inShadows + minStamina + grounded + button hold -> sent to fsm
	bool mergeTest = true;
	bool mergefall = fallingDistance > 0 && fallingDistance < maxFallingDistance;
	mergeTest &= shadow_oracle->is_shadow;
	mergeTest &= stamina > minStamina;
	mergeTest &= !isInhibited;

	// If we are not merged.
	if (!isMerged) {
		mergeTest &= stamina > minStaminaChange;
		mergeTest &= EngineInput["btShadowMerging"].hasChanged();

		TMsgSetFSMVariable onFallMsg;
		onFallMsg.variant.setName("onFallMerge");
		mergefall &= mergeTest;
		mergefall &= EngineInput["btShadowMerging"].isPressed();
		onFallMsg.variant.setBool(mergefall);
		e->sendMsg(onFallMsg);
	}

	mergeTest &= EngineInput["btShadowMerging"].isPressed();
	mergeTest &= isGrounded;

	// If the mergetest changed since last frame, update the fsm
	if (mergeTest != isMerged) {

		TMsgSetFSMVariable groundMsg;
		groundMsg.variant.setName("onmerge");
		groundMsg.variant.setBool(mergeTest);
		e->sendMsg(groundMsg);
		c_my_rigidbody->filters.mFilterData = isMerged == true ? pxPlayerFilterData : pxShadowFilterData;
		//c_my_rigidbody->controller->setStepOffset(0);
	}

	return mergeTest;
}

/* Players logic depending on ground state */
const bool TCompTempPlayerController::groundTest(float dt) {

	TCompRigidbody *c_my_collider = get<TCompRigidbody>();

	if (isGrounded != c_my_collider->is_grounded) {

		CEntity* e = CHandle(this).getOwner();

		TMsgSetFSMVariable groundMsg;
		groundMsg.variant.setName("onGround");
		groundMsg.variant.setBool(c_my_collider->is_grounded);
		e->sendMsg(groundMsg);

		TMsgSetFSMVariable falldead;
		falldead.variant.setName("onFallDead");
		falldead.variant.setBool(c_my_collider->is_grounded & (fallingTime > maxFallingTime) & !isMerged);
		e->sendMsg(falldead);

		TMsgSetFSMVariable hardLanded;
		hardLanded.variant.setName("onHardLanded");
		hardLanded.variant.setBool(c_my_collider->is_grounded & (fallingTime > hardFallingTime && fallingTime < maxFallingTime) & !isMerged);
		e->sendMsg(hardLanded);

		TMsgSetFSMVariable crouch;
		crouch.variant.setName("crouch");
		crouch.variant.setBool(false);
		e->sendMsg(crouch);

		//dbg("falling time %f\n", fallingTime);
		fallingTime = 0.f;
	}

	// Get distance to ground
	// Compute falling time
	if (!isGrounded) {

		fallingTime += dt;

		physx::PxRaycastHit hit;
		TCompTransform *c_my_transform = get<TCompTransform>();
		if (EnginePhysics.Raycast(c_my_transform->getPosition(), -c_my_transform->getUp(), 1000, hit, physx::PxQueryFlag::eSTATIC, PxPlayerDiscardQuery)) {
			fallingDistance = hit.distance;
		}
	}

	return c_my_collider->is_grounded;
}

/* Sets the player current stamina depending on player status */
void TCompTempPlayerController::updateStamina(float dt) {

	if (isMerged && !dbgDisableStamina) {

		// Determine stamina decreasing ratio multiplier depending on movement
		TCompRigidbody *c_my_rigidbody = get<TCompRigidbody>();
		TCompTransform *c_my_transform = get<TCompTransform>();
		float staminaMultiplier = c_my_rigidbody->lastFramePosition == c_my_transform->getPosition() ? decrStaticStamina : 1;

		// Determine stamina decreasing ratio depending on players up vector.
		if (fabs(EnginePhysics.gravity.Dot(c_my_rigidbody->GetUpVector())) < mergeAngle) {
			stamina = Clamp(stamina - (decrStaminaVertical * staminaMultiplier * dt), minStamina, maxStamina);
		}
		else {
			stamina = Clamp(stamina - (decrStaminaHorizontal * staminaMultiplier * dt), minStamina, maxStamina);
		}
	}
	else {
		stamina = Clamp(stamina + (incrStamina * dt), minStamina, maxStamina);
	}
}

/* Attack state, kills the closest enemy if true*/
void TCompTempPlayerController::attackState(float dt) {

  if (attackTimer > 0.7f) {   //TODO: Remove this. Only a fix for milestone 2
    CHandle enemy = closestEnemyToStun();

    if (enemy.isValid()) {
      TMsgEnemyStunned msg;
      msg.h_sender = CHandle(this).getOwner();
      enemy.sendMsg(msg);
    }

    attackTimer = 0.f;
    state = (actionhandler)&TCompTempPlayerController::idleState;
  }
  else {
    attackTimer += dt;
  }
}

/* Attack state, kills the closest enemy if true*/
void TCompTempPlayerController::mergeEnemy() {

	CHandle enemy = closeEnemy("stunned");

	if (enemy.isValid()) {
		TMsgPatrolShadowMerged msg;
		msg.h_sender = CHandle(this).getOwner();
		msg.h_objective = enemy;
		enemy.sendMsg(msg);
	}
}

/* Replace this for spatial index method/ trigger volume */
CHandle TCompTempPlayerController::closeEnemy(const std::string & state) {

	TCompTransform * mypos = get<TCompTransform>();


	/* Manage patrols */
	auto& handlesPatrol = CTagsManager::get().getAllEntitiesByTag(getID("patrol"));

	for (unsigned int i = 0; i < handlesPatrol.size(); i++) {
		if (!handlesPatrol[i].isValid()) continue;

		CEntity * eEnemy = handlesPatrol[i];
		TCompTransform * epos = eEnemy->get<TCompTransform>();

		if (VEC3::Distance(mypos->getPosition(), epos->getPosition()) < maxAttackDistance
			&& !epos->isInFront(mypos->getPosition())) {

			TCompAIPatrol * aipatrol = eEnemy->get<TCompAIPatrol>();
			if (state.compare("undefined") != 0) return handlesPatrol[i];
			if (!aipatrol->isStunned()) return handlesPatrol[i];
		}
	}

	auto& handlesMimetic = CTagsManager::get().getAllEntitiesByTag(getID("mimetic"));

	for (unsigned int i = 0; i < handlesMimetic.size(); i++) {
		if (!handlesMimetic[i].isValid()) continue;

		CEntity * eEnemy = handlesMimetic[i];
		TCompTransform * epos = eEnemy->get<TCompTransform>();

		if (VEC3::Distance(mypos->getPosition(), epos->getPosition()) < maxAttackDistance) {

			TCompAIMimetic * aimimetic = eEnemy->get<TCompAIMimetic>();
			if (state.compare("undefined") != 0) return handlesMimetic[i];
			if (!aimimetic->isStunned()) return handlesMimetic[i];
		}
	}

	return CHandle();
}

/* Replace this for spatial index method/ trigger volume */
CHandle TCompTempPlayerController::closestEnemyToStun() {

	TCompTransform * mypos = get<TCompTransform>();

	/* Manage patrols */
	auto& handlesPatrol = CTagsManager::get().getAllEntitiesByTag(getID("patrol"));

	for (unsigned int i = 0; i < handlesPatrol.size(); i++) {
		if (!handlesPatrol[i].isValid()) continue;

		CEntity * eEnemy = handlesPatrol[i];
		TCompTransform * epos = eEnemy->get<TCompTransform>();

		if (VEC3::Distance(mypos->getPosition(), epos->getPosition()) < maxAttackDistance
			&& !epos->isInFront(mypos->getPosition())
			&& mypos->isInFront(epos->getPosition())) {

			TCompAIPatrol * aipatrol = eEnemy->get<TCompAIPatrol>();
			if (!aipatrol->isStunned()) {
				return handlesPatrol[i];
			}
		}
	}

	auto& handlesMimetic = CTagsManager::get().getAllEntitiesByTag(getID("mimetic"));

	for (unsigned int i = 0; i < handlesMimetic.size(); i++) {
		if (!handlesMimetic[i].isValid()) continue;

		CEntity * eEnemy = handlesMimetic[i];
		TCompTransform * epos = eEnemy->get<TCompTransform>();

		if (VEC3::Distance(mypos->getPosition(), epos->getPosition()) < maxAttackDistance
			&& mypos->isInFront(epos->getPosition())) {

			TCompAIMimetic * aimimetic = eEnemy->get<TCompAIMimetic>();
			if (!aimimetic->isStunned()) {
				return handlesMimetic[i];
			}
		}
	}

	return CHandle();
}

/* Temporal function to determine our player shadow color, set this to a shader change..*/
void TCompTempPlayerController::updateShader(float dt) {

	TCompRender *c_my_render = get<TCompRender>();
  TCompEmissionController *e_controller = get<TCompEmissionController>();
	TCompShadowController * shadow_oracle = get<TCompShadowController>();

	if (isDead()){
    e_controller->blend(playerColor.colorDead, 1.f);
  }
	else if (isInhibited) {
    e_controller->blend(playerColor.colorInhib, 0.1f);
	}
	else if (shadow_oracle->is_shadow) {
    e_controller->blend(playerColor.colorMerge, 0.5f);
	}
  else {
    e_controller->blend(playerColor.colorIdle, 0.5f);
  }
}

VEC3 TCompTempPlayerController::getMotionDir(const VEC3 & front, const VEC3 & left) {

	VEC3 dir = VEC3::Zero;
	if (EngineInput["btUp"].isPressed() && EngineInput["btUp"].value > 0) {
		dir += fabs(EngineInput["btUp"].value) * front;
	}
	else if (EngineInput["btDown"].isPressed()) {
		dir += fabs(EngineInput["btDown"].value) * -front;
	}
	if (EngineInput["btRight"].isPressed() && EngineInput["btRight"].value > 0) {
		dir += fabs(EngineInput["btRight"].value) * left;
	}
	else if (EngineInput["btLeft"].isPressed()) {
		dir += fabs(EngineInput["btLeft"].value) * -left;
	}
	dir.Normalize();

	return dir;
}

/* Auxiliary functions */

void TCompTempPlayerController::upButtonReselased() {
	if (tempInverseVerticalMovementMerged) tempInverseVerticalMovementMerged = false;
}

bool TCompTempPlayerController::isDead()
{
	TCompFSM *fsm = get<TCompFSM>();
	return fsm->getStateName().compare("dead") == 0;
}

// Needed to avoid the isGround problem by now
void TCompTempPlayerController::resetMerge() {

	isMerged = true;
	CEntity* e = CHandle(this).getOwner();

	TMsgSetFSMVariable groundMsg;
	groundMsg.variant.setName("onmerge");
	groundMsg.variant.setBool(true);
	e->sendMsg(groundMsg);

	TMsgSetFSMVariable fallMsg;
	fallMsg.variant.setName("onFallDead");
	fallMsg.variant.setBool(false);
	e->sendMsg(fallMsg);

	TMsgSetFSMVariable hardFallMsg;
	hardFallMsg.variant.setName("onHardLanded");
	hardFallMsg.variant.setBool(false);
	e->sendMsg(hardFallMsg);
}
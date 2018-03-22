#include "mcv_platform.h"
#include "comp_player_tempcontroller.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"
#include "components/comp_render.h"
#include "components/physics/comp_rigidbody.h"
#include "components/player_controller/comp_shadow_controller.h"

#include "render/mesh/mesh_loader.h"

DECL_OBJ_MANAGER("player_tempcontroller", TCompTempPlayerController);

void TCompTempPlayerController::debugInMenu() {

}

void TCompTempPlayerController::load(const json& j, TEntityParseContext& ctx) {

	state = (actionhandler)&TCompTempPlayerController::idleState;

	auto pj_idle = loadMesh("data/meshes/pj_idle.mesh");
	auto pj_attack = loadMesh("data/meshes/pj_attack.mesh");
	auto pj_fall = loadMesh("data/meshes/pj_fall.mesh");
	auto pj_walk = loadMesh("data/meshes/pj_walk.mesh");
	auto pj_run = loadMesh("data/meshes/pj_run.mesh");
	auto pj_crouch = loadMesh("data/meshes/pj_crouch.mesh");
	auto pj_shadowmerge = loadMesh("data/meshes/pj_shadowmerge.mesh");

	// Insert them in the map.
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_idle", (CRenderMesh*)pj_idle));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_attack", (CRenderMesh*)pj_attack));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_fall", (CRenderMesh*)pj_fall));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_walk", (CRenderMesh*)pj_walk));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_run", (CRenderMesh*)pj_run));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_crouch", (CRenderMesh*)pj_crouch));
	mesh_states.insert(std::pair<std::string, CRenderMesh*>("pj_shadowmerge", (CRenderMesh*)pj_shadowmerge));

	physx::PxFilterData pxFilterData;
	pxFilterData.word1 = FilterGroup::Scenario;
	shadowMergeFilter.data = pxFilterData;

	physx::PxFilterData pxFilterData;
	pxFilterData.word1 = FilterGroup::Scenario;
	playerFilter.data = pxFilterData;
}

void TCompTempPlayerController::update(float dt) {

	(this->*state)(dt);
	shadowState();
}

void TCompTempPlayerController::registerMsgs() {

	DECL_MSG(TCompTempPlayerController, TMsgChangeState, onChangeState);
}

void TCompTempPlayerController::onChangeState(const TMsgChangeState& msg){

	state = msg.action_state;

	/* Temp change of player mesh*/
	TCompRender *c_my_render = get<TCompRender>();
	c_my_render->meshes[0].mesh = mesh_states.find(msg.meshname)->second;
	c_my_render->refreshMeshesInRenderManager();

	// Change the current state.
}

void TCompTempPlayerController::idleState(float dt){

}

/* Main thirdperson player motion movement handled here */
void TCompTempPlayerController::playerMotion(float dt){

	// Player movement and rotation related method.
	float yaw, pitch, roll;
	CEntity *player_camera = (CEntity *)getEntityByName("TPCamera");
	TCompTransform *c_my_transform = get<TCompTransform>();
	TCompTransform * trans_camera = player_camera->get<TCompTransform>();
	c_my_transform->getYawPitchRoll(&yaw, &pitch, &roll);

	VEC3 dir = VEC3::Zero;
	float inputSpeed = Clamp(fabs(EngineInput["Horizontal"].value) + fabs(EngineInput["Vertical"].value), 0.f, 1.f);
	float player_accel = inputSpeed * currentSpeed * dt;

	VEC3 up = trans_camera->getFront();
	VEC3 normal_norm = c_my_transform->getUp();
	VEC3 proj = projectVector(up, normal_norm);

	if (EngineInput["btUp"].isPressed() && EngineInput["btUp"].value > 0) {
		dir += fabs(EngineInput["btUp"].value) * proj;
	}
	else if (EngineInput["btDown"].isPressed()) {
		dir += fabs(EngineInput["btDown"].value) * -proj;
	}
	if (EngineInput["btRight"].isPressed() && EngineInput["btRight"].value > 0) {
		dir += fabs(EngineInput["btRight"].value) * -normal_norm.Cross(proj);
	}
	else if (EngineInput["btLeft"].isPressed()) {
		dir += fabs(EngineInput["btLeft"].value)  * normal_norm.Cross(proj);
	}
	dir.Normalize();

	float dir_yaw = getYawFromVector(dir);
	Quaternion my_rotation = c_my_transform->getRotation();
	Quaternion new_rotation = Quaternion::CreateFromYawPitchRoll(dir_yaw, pitch, 0);
	Quaternion quat = Quaternion::Lerp(my_rotation, new_rotation, rotationSpeed * dt);
	if(dir != VEC3::Zero) c_my_transform->setRotation(quat);
	c_my_transform->setPosition(c_my_transform->getPosition() + dir * player_accel);
}

void TCompTempPlayerController::playerShadowMotion(float dt) {

	// Player movement and rotation related method.
	CEntity *player_camera = (CEntity *)getEntityByName("SMCameraHor");

	TCompRender *c_my_render = get<TCompRender>();
	TCompCollider * collider = get<TCompCollider>();
	TCompRigidbody * rigidbody = get<TCompRigidbody>();
	TCompTransform *c_my_transform = get<TCompTransform>();
	TCompTransform * trans_camera = player_camera->get<TCompTransform>();

	float inputSpeed = Clamp(fabs(EngineInput["Horizontal"].value) + fabs(EngineInput["Vertical"].value), 0.f, 1.f);
	float player_accel = inputSpeed * currentSpeed * dt;
	
	VEC3 dir = VEC3::Zero;
	VEC3 up = trans_camera->getUp();
	VEC3 normal_norm = rigidbody->normal_gravity;
	VEC3 proj = projectVector(up, normal_norm);

	if (EngineInput["btUp"].isPressed() && EngineInput["btUp"].value > 0) {
		dir += fabs(EngineInput["btUp"].value) * proj;
	}
	else if (EngineInput["btDown"].isPressed()) {
		dir += fabs(EngineInput["btDown"].value) * -proj;
	}
	if (EngineInput["btRight"].isPressed() && EngineInput["btRight"].value > 0) {
		dir += fabs(EngineInput["btRight"].value) * normal_norm.Cross(proj);
	}
	else if (EngineInput["btLeft"].isPressed()) {
		dir += fabs(EngineInput["btLeft"].value) * -normal_norm.Cross(proj);
	}
	dir.Normalize();

	if (dir != VEC3::Zero)
	{
		VEC3 new_pos = c_my_transform->getPosition() - dir;
		Matrix test = Matrix::CreateLookAt(c_my_transform->getPosition(), new_pos, c_my_transform->getUp()).Transpose();
		Quaternion quat = Quaternion::CreateFromRotationMatrix(test);
		c_my_transform->setRotation(quat);
	}

	VEC3 new_pos = c_my_transform->getPosition() + dir * player_accel;
	c_my_transform->setPosition(new_pos);
}

const bool TCompTempPlayerController::ConcaveTest(void)
{
	physx::PxRaycastHit hit;
	TCompCollider *c_my_collider = get<TCompCollider>();
	TCompRigidbody *rigidbody = get<TCompRigidbody>();
	TCompTransform *c_my_transform = get<TCompTransform>();
	VEC3 upwards_offset = c_my_transform->getPosition() + c_my_transform->getUp() * .01f;

	if (EnginePhysics.Raycast(upwards_offset, c_my_transform->getFront(), 0.35f + .1f, hit, physx::PxQueryFlag::eSTATIC, playerFilter))
	{
		VEC3 hit_normal = VEC3(hit.normal.x, hit.normal.y, hit.normal.z);
		VEC3 hit_point = VEC3(hit.position.x, hit.position.y, hit.position.z);
		if (hit_normal == c_my_transform->getUp()) return false;

		if (EnginePhysics.gravity.Dot(hit_normal) < .01f)
		{
			VEC3 new_forward = hit_normal.Cross(c_my_transform->getLeft());
			VEC3 target = c_my_transform->getPosition() + new_forward;

			rigidbody->SetUpVector(hit_normal);
			rigidbody->normal_gravity = 9.8f * -hit_normal;

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

const bool TCompTempPlayerController::ConvexTest(void)
{
	physx::PxRaycastHit hit;
	TCompCollider *c_my_collider = get<TCompCollider>();
	TCompTransform *c_my_transform = get<TCompTransform>();
	TCompRigidbody *rigidbody = get<TCompRigidbody>();
	VEC3 upwards_offset = c_my_transform->getPosition() + c_my_transform->getUp() * .01f;
	VEC3 new_dir = c_my_transform->getUp() + c_my_transform->getFront();
	new_dir.Normalize();

	if (EnginePhysics.Raycast(upwards_offset, -new_dir, 0.65f, hit, physx::PxQueryFlag::eSTATIC, playerFilter))
	{
		VEC3 hit_normal = VEC3(hit.normal.x, hit.normal.y, hit.normal.z);
		VEC3 hit_point = VEC3(hit.position.x, hit.position.y, hit.position.z);
		if (hit_normal == c_my_transform->getUp()) return false;

		if (hit.distance > .015f && EnginePhysics.gravity.Dot(hit_normal) < .01f)
		{
			VEC3 new_forward = -hit_normal.Cross(c_my_transform->getLeft());
			VEC3 target = hit_point + new_forward;

			rigidbody->SetUpVector(hit_normal);
			rigidbody->normal_gravity = 9.8f * -hit_normal;

			QUAT new_rotation = createLookAt(hit_point, target, hit_normal);
			VEC3 new_pos = hit_point + 0.35f * new_forward;
			c_my_transform->setRotation(new_rotation);
			c_my_transform->setPosition(new_pos);
			return true;
		}
	}

	return false;
}

/* Temporal function to determine our player shadow color, set this to a shader change..*/
void TCompTempPlayerController::shadowState() {

	TCompRender *c_my_render = get<TCompRender>();
	TCompShadowController * shadow_oracle = get<TCompShadowController>();

	if (c_my_render->color == VEC4(1, 1, 1, 1) || c_my_render->color == VEC4(0, .8f, 1, 1)) {
		if (shadow_oracle->is_shadow) c_my_render->color = Color(0, .8f, 1);
		else c_my_render->color = Color(1, 1, 1);
	}
}
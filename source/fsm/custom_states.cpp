#include "mcv_platform.h"
#include "custom_states.h"
#include "context.h"
#include "components/player_controller/comp_player_animator.h"
#include "components/physics/comp_rigidbody.h"
//class TCompTempPlayerController;
//class TCompPlayerAnimator;
//
//typedef void (TCompTempPlayerController::*actionhandler)(float);
//typedef void (TCompPlayerAnimator::*animatonhandler)(float);

namespace FSM
{
	TargetCamera * getTargetCamera(const json& jData) {

		TargetCamera * target = new TargetCamera();
		target->name = jData["target"];
		target->blendIn = jData.value("blendIn", 0.01f);
		target->blendOut = jData.value("blendOut", 0.01f);

		return target;
	}

	Noise * getNoise(const json& jData) {
		Noise * noise = new Noise();
		if (jData != NULL) {
			noise->isNoise = true;
			noise->isOnlyOnce = jData.value("is_only_once", false);
			noise->noiseRadius = jData.value("radius", 0.01f);
			noise->timeToRepeat = jData.value("time_to_repeat", 1.f);
			noise->isArtificial = jData.value("is_artificial", false);
		}
		else {
			noise->isNoise = false;
			noise->isOnlyOnce = false;
			noise->noiseRadius = 0.01f;
			noise->timeToRepeat = 1.f;
			noise->isArtificial = false;
		}

		return noise;
	}



	bool IdleState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 4.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void IdleState::onStart(CContext& ctx) const {

		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ _animationName });
		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::IDLE , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, nullptr, _noise });
	}

	void IdleState::onFinish(CContext& ctx) const {

	}

	bool WalkState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 4.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_rotation_speed = jData.value("rotationSpeed", 10.f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void WalkState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "walk" });

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::WALK , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, nullptr, _noise });
	}

	void WalkState::onFinish(CContext& ctx) const {

	}

	bool RunState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 5.5f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_rotation_speed = jData.value("rotationSpeed", 10.f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void RunState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "run" });

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::RUN , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, nullptr, _noise });
	}

	void RunState::onFinish(CContext& ctx) const {

	}

	bool FallState::load(const json& jData) {

		_force = jData.value("force", 1.f);
		_speed = jData.value("speed", 3.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_animationName = jData["animation"];
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void FallState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::FALL , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, nullptr, _noise });
	}

	void FallState::onFinish(CContext& ctx) const {

	}


	bool CrouchState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 3.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_rotation_speed = jData.value("rotationSpeed", 10.f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void CrouchState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "crouch" });

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::CROUCH_IDLE , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, nullptr, _noise });
	}

	void CrouchState::onFinish(CContext& ctx) const {

	}

	bool CrouchWalkState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 3.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_rotation_speed = jData.value("rotationSpeed", 10.f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void CrouchWalkState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "crouch" });

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::CROUCH_WALK , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, nullptr, _noise });
	}

	void CrouchWalkState::onFinish(CContext& ctx) const {

	}

	bool EnterMergeState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 3.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void EnterMergeState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "crouch" });

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::SM_POSE , 1.0f });
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::SM_ENTER , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });

		// Hardcoded for testing purposes, move this out of here in the future
		//TCompTempPlayerController * t_comp = e->get<TCompTempPlayerController>();
		//TCompTransform * t_trans = e->get<TCompTransform>();
		//TCompRigidbody * t_rigid = e->get<TCompRigidbody>();
		//physx::PxCapsuleController* caps = (physx::PxCapsuleController*)t_rigid->controller;
		//caps->setRadius(0.1f);
		//caps->resize(0.1f);
		//caps->setFootPosition(physx::PxExtendedVec3(t_trans->getPosition().x, t_trans->getPosition().y, t_trans->getPosition().z));
	}

	void EnterMergeState::onFinish(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::mergeEnemy });
	}

	bool MergeState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 3.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void MergeState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "crouch" });

		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::mergeState, _speed, _size, _radius, nullptr, _noise });
	}

	void MergeState::onFinish(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::resetState });
		dbg("reset stated\n");
	}

	bool ExitMergeState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 3.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void ExitMergeState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "crouch" });

		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });

		// Disable the rigidbody so that we can handle our transition in air manually
		// Hardcoded for testing purposes, move this out of here in the future
		//TCompTempPlayerController * t_comp = e->get<TCompTempPlayerController>();
		//TCompRigidbody * t_rigid = e->get<TCompRigidbody>();
		//physx::PxCapsuleController* caps = (physx::PxCapsuleController*)t_rigid->controller;
		//caps->setRadius(0.3f);
		//caps->setHeight(1.f);
	}

	void ExitMergeState::onFinish(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::exitMergeState });
		// Re enable rigidbody.

		// Hardcoded for testing purposes, move this out of here in the future
		//TCompTempPlayerController * t_comp = e->get<TCompTempPlayerController>();
		//TCompTransform * t_trans = e->get<TCompTransform>();
		//TCompRigidbody * t_rigid = e->get<TCompRigidbody>();
		//physx::PxCapsuleController* caps = (physx::PxCapsuleController*)t_rigid->controller;
		//caps->setRadius(0.3f);
		//caps->setFootPosition(physx::PxExtendedVec3(t_trans->getPosition().x, t_trans->getPosition().y, t_trans->getPosition().z));
	}


	bool LandMergeState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 2.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void LandMergeState::onStart(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::LAND_SOFT , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState,_speed, _size, _radius, _target, _noise });
	}
	void LandMergeState::onFinish(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::resetMerge });
	}


	bool SoftLandState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 2.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void SoftLandState::onStart(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::LAND_SOFT , 1.0f });
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::IDLE , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
	}
	void SoftLandState::onFinish(CContext& ctx) const {

	}

	bool HardLandState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 2.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void HardLandState::onStart(CContext& ctx) const {
		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::LAND_SOFT , 1.0f });
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::IDLE , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
	}

	void HardLandState::onFinish(CContext& ctx) const {

	}


	bool AttackState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 2.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void AttackState::onStart(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::ATTACK , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::attackState, _speed, _radius, _size, nullptr, _noise });
	}
	void AttackState::onFinish(CContext& ctx) const {

	}

	bool RemoveInhibitor::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 2.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void RemoveInhibitor::onStart(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::removingInhibitorState, _speed, _radius, _size, nullptr, _noise });

	}

	void RemoveInhibitor::onFinish(CContext& ctx) const {

	}

	bool InhibitorRemoved::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 2.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void InhibitorRemoved::onStart(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::METRALLA_FINISH , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _radius, _size, nullptr, _noise });

	}

	void InhibitorRemoved::onFinish(CContext& ctx) const {

	}

	bool InhibitorTryToRemove::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 2.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void InhibitorTryToRemove::onStart(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::METRALLA_MIDDLE , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _radius, _size, nullptr, _noise });

	}

	void InhibitorTryToRemove::onFinish(CContext& ctx) const {

	}

	bool DeadState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 2.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
		return true;
	}

	void DeadState::onStart(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::DEATH , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::deadState, _speed, _radius, _size, nullptr, _noise });
	}
	void DeadState::onFinish(CContext& ctx) const {

	}
}
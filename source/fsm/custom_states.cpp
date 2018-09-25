#include "mcv_platform.h"
#include "custom_states.h"
#include "context.h"
#include "components/player_controller/comp_player_animator.h"
#include "components/physics/comp_rigidbody.h"
#include "components/lighting/comp_projector.h"
#include "components/comp_render.h"
#include "components/comp_particles.h"
#include "components/player_controller/comp_shadow_controller.h"

// Refactor this after Milestone3, move everything unnecessary to player class
namespace FSM
{
    TargetCamera * getTargetCamera(const json& jData) {

        TargetCamera * target = new TargetCamera();
        target->name = jData["target"];
        target->blendIn = jData.value("blendIn", 0.01f);
        target->blendOut = jData.value("blendOut", 0.01f);
        target->fov = jData.value("fov", 70.f);

        return target;
    }

    Noise * getNoise(const json& jData) {
        Noise * noise = new Noise();
        if (jData != NULL) {
            noise->isNoise = true;
            noise->isOnlyOnce = jData.value("is_only_once", false);
            noise->noiseRadius = jData.value("radius", -1.f);
            noise->timeToRepeat = jData.value("time_to_repeat", 1.f);
            noise->isArtificial = jData.value("is_artificial", false);
        }
        else {
            noise->isNoise = false;
            noise->isOnlyOnce = false;
            noise->noiseRadius = -1.f;
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
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void IdleState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::IDLE , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
        TCompTempPlayerController * playerController = e->get<TCompTempPlayerController>();
        playerController->resetRemoveInhibitor();
    }

    void IdleState::onFinish(CContext& ctx) const {
        CEntity* e = ctx.getOwner();
        TCompTempPlayerController * playerController = e->get<TCompTempPlayerController>();
        playerController->canRemoveInhibitor = false;

    }

    bool WalkState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 4.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _rotation_speed = jData.value("rotationSpeed", 10.f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void WalkState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::WALK , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, _target, _noise });

        // Disable the players projector.
        CHandle player_light = getEntityByName("LightPlayer");
        if (player_light.isValid()) {
            CEntity * entity_light = (CEntity*)player_light;
            TCompProjector * light = entity_light->get<TCompProjector>();
            light->isEnabled = false;
        }
    }

    void WalkState::onFinish(CContext& ctx) const {

    }

    bool WalkSlowState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 4.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _rotation_speed = jData.value("rotationSpeed", 10.f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void WalkSlowState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::WALK_SLOW , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, _target, _noise });
    }

    void WalkSlowState::onFinish(CContext& ctx) const {

    }

    bool RunState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 5.5f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _rotation_speed = jData.value("rotationSpeed", 10.f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void RunState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::RUN , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, _target, _noise });
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
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void FallState::onStart(CContext& ctx) const {

        // Send a message to the player controller
        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::FALL , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::fallState, _speed, _size, _radius, _target, _noise });
    }

    void FallState::onFinish(CContext& ctx) const {

    }

    bool FallDieState::load(const json& jData) {

        _force = jData.value("force", 1.f);
        _speed = jData.value("speed", 3.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _animationName = jData["animation"];
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void FallDieState::onStart(CContext& ctx) const {

        // Send a message to the player controller
        CEntity* e = ctx.getOwner();
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
    }

    void FallDieState::onFinish(CContext& ctx) const {
        CEntity* e = ctx.getOwner();
        e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::die });
    }


    bool CrouchState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 3.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _rotation_speed = jData.value("rotationSpeed", 10.f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void CrouchState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::CROUCH_IDLE , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
        TCompTempPlayerController * playerController = e->get<TCompTempPlayerController>();
        playerController->resetRemoveInhibitor();
    }

    void CrouchState::onFinish(CContext& ctx) const {
        CEntity* e = ctx.getOwner();
        TCompTempPlayerController * playerController = e->get<TCompTempPlayerController>();
        playerController->canRemoveInhibitor = false;
    }

    bool SonarStateCrouch::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 3.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _rotation_speed = jData.value("rotationSpeed", 10.f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;

        return true;
    }

    void SonarStateCrouch::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::SONDA_CROUCH , 1.0f });        
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
        e->sendMsg(TMsgSonarActive{ 1.f });
    }

    void SonarStateCrouch::onFinish(CContext& ctx) const {

    }

	bool SonarStateUp::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 3.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_rotation_speed = jData.value("rotationSpeed", 10.f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		_target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;

		return true;
	}

	void SonarStateUp::onStart(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::SONDA_NORMAL , 1.0f });
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
		e->sendMsg(TMsgSonarActive{ 1.f });
	}

	void SonarStateUp::onFinish(CContext& ctx) const {

	}

    bool CrouchWalkState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 3.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _rotation_speed = jData.value("rotationSpeed", 10.f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;

        return true;
    }

    void CrouchWalkState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::CROUCH_WALK , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, _target, _noise });
    }

    void CrouchWalkState::onFinish(CContext& ctx) const {

    }

    bool CrouchWalkSlowState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 3.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _rotation_speed = jData.value("rotationSpeed", 10.f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void CrouchWalkSlowState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::CROUCH_WALK_SLOW , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, _target, _noise });
    }

    void CrouchWalkSlowState::onFinish(CContext& ctx) const {

    }

    bool EnterMergeState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 3.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;

        return true;
    }

    void EnterMergeState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::SM_POSE , 1.0f });
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::SM_ENTER , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::mergeState, _speed, _size, _radius, _target, _noise });
        e->sendMsg(TMsgFadeBody{ false });

        //// Testing!
        CHandle player_light = getEntityByName("LightPlayer");
        if (player_light.isValid()) {
            CEntity * entity_light = (CEntity*)player_light;
            TCompProjector * light = entity_light->get<TCompProjector>();
            light->isEnabled = true;
        }

        // Move this to LUA in the future.
        Engine.get().getParticles().launchSystem("data/particles/sm_enter_expand.particles", ctx.getOwner());
        Engine.get().getParticles().launchSystem("data/particles/sm_enter_splash2.particles", ctx.getOwner());
        Engine.get().getParticles().launchSystem("data/particles/sm_enter_sparks.particles", ctx.getOwner());

        TCompParticles * c_e_particle = e->get<TCompParticles>();
        c_e_particle->setSystemState(true);

        CEntity * ent = getEntityByName("Player_Idle_SM");
        TCompParticles * c_e_particle2 = ent->get<TCompParticles>();
        assert(c_e_particle2);
        c_e_particle2->setSystemState(false);

        EngineLogic.execScript("animation_enter_merge()");
    }

    void EnterMergeState::onFinish(CContext& ctx) const {

        CEntity* e = ctx.getOwner();

        TCompRender * render = e->get<TCompRender>();
        render->visible = false;
        e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::resetMergeFall });
    }

    bool MergeState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 3.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void MergeState::onStart(CContext& ctx) const {

        // Send a message to the player controller
        CEntity* e = ctx.getOwner();
        //e->sendMsg(TMsgAnimation{ "crouch" });

        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::mergeState, _speed, _size, _radius, _target, _noise });
    }

    void MergeState::onFinish(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::resetState });
    }

    bool ExitMergeInterruptedState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 3.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void ExitMergeInterruptedState::onStart(CContext& ctx) const {

        // Send a message to the player controller
        CEntity* e = ctx.getOwner();
        //e->sendMsg(TMsgAnimation{ "crouch" });
		TCompPlayerAnimator *c_animator = e->get<TCompPlayerAnimator>();
		if(c_animator->isPlayingAnimation((TCompAnimator::EAnimation)TCompPlayerAnimator::EAnimation::SM_ENTER))
			c_animator->removeAction((TCompAnimator::EAnimation)TCompPlayerAnimator::EAnimation::SM_ENTER);
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::mergeState, _speed, _size, _radius, _target, _noise });
    }

    void ExitMergeInterruptedState::onFinish(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::resetState });
    }

    bool ExitMergeState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 3.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;

        return true;
    }

    void ExitMergeState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, _target, _noise });
        e->sendMsg(TMsgFadeBody{ true });

        // Testing!
        CHandle player_light = getEntityByName("LightPlayer");
        if (player_light.isValid()) {
            CEntity * entity_light = (CEntity*)player_light;
            TCompProjector * light = entity_light->get<TCompProjector>();
            light->isEnabled = false;
        }

        // TO REFACTOR
        // Sets particles and calls the finishing state.
        Engine.get().getParticles().launchSystem("data/particles/sm_enter_expand.particles", ctx.getOwner());
        Engine.get().getParticles().launchSystem("data/particles/sm_enter_splash.particles", ctx.getOwner());
        Engine.get().getParticles().launchSystem("data/particles/sm_enter_sparks.particles", ctx.getOwner());

        TCompParticles * c_e_particle = e->get<TCompParticles>();
        c_e_particle->setSystemState(false);

        EngineLogic.execScript("animation_exit_merge()");
    }

    void ExitMergeState::onFinish(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::exitMergeState });

        TCompRender * render = e->get<TCompRender>();
        render->visible = true;

        TCompShadowController * shadow = e->get<TCompShadowController>();
        CEntity * ent = getEntityByName("Player_Idle_SM");
        TCompParticles * c_e_particle2 = ent->get<TCompParticles>();
        assert(c_e_particle2);
        c_e_particle2->setSystemState(shadow->is_shadow);
    }

	bool ExitMergeCrouchedState::load(const json & jData)
	{
		_animationName = jData["animation"];
		_speed = jData.value("speed", 3.f);
		_size = jData.value("size", 1.f);
		_radius = jData.value("radius", 0.3f);
		_noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
		_target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;

		return true;
	}

	void ExitMergeCrouchedState::onStart(CContext & ctx) const
	{
		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::walkState, _speed, _size, _radius, _target, _noise });
        e->sendMsg(TMsgFadeBody{ true });

		// Testing!
		CHandle player_light = getEntityByName("LightPlayer");
		if (player_light.isValid()) {
			CEntity * entity_light = (CEntity*)player_light;
			TCompProjector * light = entity_light->get<TCompProjector>();
			light->isEnabled = false;
		}

        // TO REFACTOR
        // Sets particles and calls the finishing state.
        Engine.get().getParticles().launchSystem("data/particles/sm_enter_expand.particles", ctx.getOwner());
        Engine.get().getParticles().launchSystem("data/particles/sm_enter_splash.particles", ctx.getOwner());
        Engine.get().getParticles().launchSystem("data/particles/sm_enter_sparks.particles", ctx.getOwner());

        TCompParticles * c_e_particle = e->get<TCompParticles>();
        c_e_particle->setSystemState(false);

        EngineLogic.execScript("animation_exit_merge()");
	}

	void ExitMergeCrouchedState::onFinish(CContext & ctx) const
	{
		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::exitMergeState });
		// Re enable rigidbody.

		TCompRender * render = e->get<TCompRender>();
		render->visible = true;

        TCompShadowController * shadow = e->get<TCompShadowController>();
        CEntity * ent = getEntityByName("Player_Idle_SM");
        TCompParticles * c_e_particle2 = ent->get<TCompParticles>();
        assert(c_e_particle2);
        c_e_particle2->setSystemState(shadow->is_shadow);
	}

    bool LandMergeState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 2.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void LandMergeState::onStart(CContext& ctx) const {

        // Send a message to the player controller
        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::FALL , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::mergeFallState, _speed, _size, _radius, _target, _noise });

        //CEntity* e = ctx.getOwner();
        //e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::LAND_SOFT , 1.0f });
        //e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState,_speed, _size, _radius, _target, _noise });

        //CHandle player_light = getEntityByName("LightPlayer");
        //if (player_light.isValid()) {
        //    CEntity * entity_light = (CEntity*)player_light;
        //    TCompProjector * light = entity_light->get<TCompProjector>();
        //    light->isEnabled = true;
        //}

        //// Move all of this to LUA
        //Engine.get().getParticles().launchSystem("data/particles/sm_enter_expand.particles", ctx.getOwner());
        //Engine.get().getParticles().launchSystem("data/particles/sm_enter_splash.particles", ctx.getOwner());
        //Engine.get().getParticles().launchSystem("data/particles/sm_enter_sparks.particles", ctx.getOwner());

        //TCompParticles * c_e_particle = e->get<TCompParticles>();
        //c_e_particle->setSystemState(true);

        //TCompRender * render = e->get<TCompRender>();
        //render->visible = false;
    }

    void LandMergeState::onFinish(CContext& ctx) const {

        //CEntity* e = ctx.getOwner();
        //e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::resetMerge });
    }


    bool SoftLandState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 2.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void SoftLandState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::LAND_SOFT , 1.0f });
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::IDLE , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });

        EngineLogic.execScript("animation_soft_land()");
    }

    void SoftLandState::onFinish(CContext& ctx) const {

    }

    bool HardLandState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 2.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void HardLandState::onStart(CContext& ctx) const {
        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::LAND_HARD , 1.0f });
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::IDLE , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
        TCompTempPlayerController * playerController = e->get<TCompTempPlayerController>();
        playerController->getDamage(30.f);
        EngineLogic.execScript("animation_hard_land()");
    }

    void HardLandState::onFinish(CContext& ctx) const {

    }

    bool AttackState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 2.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;

        return true;
    }

    void AttackState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::ATTACK , 1.0f });
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::IDLE , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
    }

    void AttackState::onFinish(CContext& ctx) const {

    }

    bool InhibitorRemovedState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 2.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void InhibitorRemovedState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::METRALLA_FINISH , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });

    }

    void InhibitorRemovedState::onFinish(CContext& ctx) const {

    }

    bool InhibitorTryToRemoveState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 2.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void InhibitorTryToRemoveState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::METRALLA_MIDDLE , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::removingInhibitorState, _speed, _size, _radius, _target, _noise });
        TCompTempPlayerController * playerController = e->get<TCompTempPlayerController>();
        playerController->canRemoveInhibitor = true;

    }

    void InhibitorTryToRemoveState::onFinish(CContext& ctx) const {
        CEntity* e = ctx.getOwner();
        TCompTempPlayerController * playerController = e->get<TCompTempPlayerController>();
        //playerController->canRemoveInhibitor = true;
    }

    bool DieState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 2.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void DieState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::DEATH , 1.0f });
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::DEAD , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
    }

    void DieState::onFinish(CContext& ctx) const {

    }

    bool DeadState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 2.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        _target = jData.count("camera") ? getTargetCamera(jData["camera"]) : nullptr;
        return true;
    }

    void DeadState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::DEAD , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
    }

    void DeadState::onFinish(CContext& ctx) const {

    }

    bool GrabEnemyState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 2.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
        return true;
    }

    void GrabEnemyState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::GRAB_ENEMY , 1.0f });
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::GRABING_ENEMY , 1.0f });
        e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });

    }

    void GrabEnemyState::onFinish(CContext& ctx) const {
        CEntity* e = ctx.getOwner();
        e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::mergeEnemy });
    }

    bool MovingObjectState::load(const json& jData) {

      _animationName = jData["animation"];
      _speed = jData.value("speed", 2.f);
      _size = jData.value("size", 1.f);
      _radius = jData.value("radius", 0.3f);
      _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
      if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
      return true;
    }

    void MovingObjectState::onStart(CContext& ctx) const {

      CEntity* e = ctx.getOwner();
      e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::IDLE , 1.0f });
      TCompTempPlayerController * playerController = e->get<TCompTempPlayerController>();
      playerController->markObjectAsMoving(true);
      e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::movingObjectState, _speed, _size, _radius, _target, _noise });
    }

    void MovingObjectState::onFinish(CContext& ctx) const {
      CEntity* e = ctx.getOwner();
      TCompTempPlayerController * playerController = e->get<TCompTempPlayerController>();
      playerController->markObjectAsMoving(false);
      //e->sendMsg(TMsgStateFinish{ (actionfinish)&TCompTempPlayerController::markObjectAsMoving });
    }

    bool PressingButtonState::load(const json& jData) {

        _animationName = jData["animation"];
        _speed = jData.value("speed", 2.f);
        _size = jData.value("size", 1.f);
        _radius = jData.value("radius", 0.3f);
        _noise = jData.count("noise") ? getNoise(jData["noise"]) : getNoise(NULL);
        if (jData.count("camera")) _target = getTargetCamera(jData["camera"]);
        return true;
    }

    void PressingButtonState::onStart(CContext& ctx) const {

        CEntity* e = ctx.getOwner();
        e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::METRALLA_FINISH , 1.0f });
        //e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::IDLE , 1.0f });
        //e->sendMsg(TMsgStateStart{ (actionhandler)&TCompTempPlayerController::idleState, _speed, _size, _radius, _target, _noise });
    }

    void PressingButtonState::onFinish(CContext& ctx) const {

    }
}
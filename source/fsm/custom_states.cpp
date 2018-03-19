#include "mcv_platform.h"
#include "custom_states.h"
#include "context.h"

#include "components/player_controller/comp_fake_animation_controller.h"

namespace FSM
{

	bool AnimationState::load(const json& jData){

		_animationName = jData["animation"];

		return true;
	}

	void AnimationState::onStart(CContext& ctx) const {

		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgAnimation{ _animationName });
	}

	void AnimationState::onFinish(CContext& ctx) const {

	}

	bool JumpState::load(const json& jData) {

		_force = jData.value("force", 1.f);

		return true;
	}

	void JumpState::onStart(CContext& ctx) const {

		// Send a message to the player controller

	}

	void JumpState::onFinish(CContext& ctx) const {

	}

	bool HitState::load(const json& jData) {

		// ..
		return true;
	}

	void HitState::onStart(CContext& ctx) const {

		// ..
	}
	void HitState::onFinish(CContext& ctx) const {

	}

}
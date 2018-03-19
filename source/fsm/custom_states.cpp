#include "mcv_platform.h"
#include "custom_states.h"
#include "context.h"

#include "components/player_controller/comp_fake_animation_controller.h"

namespace FSM
{
  void AnimationState::onStart(CContext& ctx) const
  {
    CEntity* e = ctx.getOwner();
    e->sendMsg(TMsgAnimation{ _animationName });
  }

  bool AnimationState::load(const json& jData)
  {
    _animationName = jData["animation"];

    return true;
  }


  void JumpState::onStart(CContext& ctx) const
  {
    // 
  }

  bool JumpState::load(const json& jData)
  {
    _force = jData.value("force", 1.f);

    return true;
  }


  void HitState::onStart(CContext& ctx) const
  {
    // ..
  }
  bool HitState::load(const json& jData)
  {
    // ..

    return true;
  }
}
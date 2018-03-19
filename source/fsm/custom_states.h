#pragma once

#include "state.h"

namespace FSM
{
  class AnimationState : public IState
  {
    virtual void onStart(CContext& ctx) const override;
    virtual bool load(const json& jData);

  private:
    std::string _animationName;
  };

  class JumpState : public IState
  {
    virtual void onStart(CContext& ctx) const override;
    virtual bool load(const json& jData);

  private:
    float _force = 1.f;
  };

  class HitState : public IState
  {
    virtual void onStart(CContext& ctx) const override;
    virtual bool load(const json& jData);
  };
}

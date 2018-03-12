#pragma once

#include "transition.h"

namespace FSM
{
  class VariableTransition : public ITransition
  {
    virtual bool checkCondition(CContext& ctx) const override;
    virtual bool load(const json& jData) override;
  };

  class TimeTransition : public ITransition
  {
    virtual bool checkCondition(CContext& ctx) const override;
    virtual bool load(const json& jData) override;
  };
}

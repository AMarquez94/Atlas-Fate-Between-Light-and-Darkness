#pragma once

#include "transition.h"
#include "utils/variant.h"

namespace FSM
{
  class VariableTransition : public ITransition
  {
    virtual bool checkCondition(CContext& ctx) const override;
    virtual bool load(const json& jData) override;

  private:
    enum class EOperation { EQUAL = 0, GREATER, GREATER_EQUAL };
    CVariant _variable;
    EOperation _operation = EOperation::EQUAL;
  };

  class TimeTransition : public ITransition
  {
    virtual bool checkCondition(CContext& ctx) const override;
    virtual bool load(const json& jData) override;

  private:
    float _time = 0.f;
  };
}

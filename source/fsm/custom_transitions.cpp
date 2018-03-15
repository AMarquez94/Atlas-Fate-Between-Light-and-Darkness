#include "mcv_platform.h"
#include "custom_transitions.h"

namespace FSM
{
  bool VariableTransition::checkCondition(CContext& ctx) const
  {
    // ..
    return false;
  }
  bool VariableTransition::load(const json& jData)
  {
    // ..
    return true;
  }


  bool TimeTransition::checkCondition(CContext& ctx) const
  {
    // ..
    return false;
  }
  bool TimeTransition::load(const json& jData)
  {
    // ..
    return true;
  }
}
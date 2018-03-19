#include "mcv_platform.h"
#include "custom_transitions.h"
#include "context.h"

namespace FSM
{
  bool VariableTransition::checkCondition(CContext& ctx) const
  {
    const CVariant* var = ctx.getVariable(_variable.getName());
    if (var && var->getType() == _variable.getType())
    {
      if (var->getType() == CVariant::EType::BOOL)
      {
        return var->getBool() == _variable.getBool();
      }
      else if (var->getType() == CVariant::EType::INT)
      {
        return var->getInt() == _variable.getInt();
      }
      else if (var->getType() == CVariant::EType::FLOAT)
      {
        return var->getFloat() == _variable.getFloat();
      }
      else if (var->getType() == CVariant::EType::HANDLE)
      {
        return var->getHandle() == _variable.getHandle();
      }
      else if (var->getType() == CVariant::EType::STRING)
      {
        return var->getString() == _variable.getString();
      }
    }
    return false;
  }

  bool VariableTransition::load(const json& jData)
  {
    _variable.load(jData["variable"]);
    const std::string& opType = jData.value("operation", std::string("equal"));
    if (opType == "greater")
    {
      _operation = EOperation::GREATER;
    }
    else if (opType == "greater_equal")
    {
      _operation = EOperation::GREATER_EQUAL;
    }
    else
    {
      _operation = EOperation::EQUAL;
    }
    return true;
  }


  bool TimeTransition::checkCondition(CContext& ctx) const
  {
    return ctx.getTimeInState() >= _time;
  }
  bool TimeTransition::load(const json& jData)
  {
    _time = jData.value("time", 0.f);
    return true;
  }
}
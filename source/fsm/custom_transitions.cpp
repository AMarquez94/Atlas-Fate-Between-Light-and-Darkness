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
		if (_operation == EOperation::EQUAL) {
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

		if (_operation == EOperation::GREATER) {

			if (var->getType() == CVariant::EType::INT)
			{
				return var->getInt() > _variable.getInt();
			}
			else if (var->getType() == CVariant::EType::FLOAT)
			{
				return var->getFloat() > _variable.getFloat();
			}
		}
		
		if (_operation == EOperation::GREATER_EQUAL) {
			if (var->getType() == CVariant::EType::INT)
			{
				return var->getInt() >= _variable.getInt();
			}
			else if (var->getType() == CVariant::EType::FLOAT)
			{
				return var->getFloat() >= _variable.getFloat();
			}
		}

		if (_operation == EOperation::LOWER) {
			if (var->getType() == CVariant::EType::INT)
			{
				return var->getInt() < _variable.getInt();
			}
			else if (var->getType() == CVariant::EType::FLOAT)
			{
				return var->getFloat() < _variable.getFloat();
			}
		}

		if (_operation == EOperation::LOWER_EQUAL) {
			if (var->getType() == CVariant::EType::INT)
			{
				return var->getInt() <= _variable.getInt();
			}
			else if (var->getType() == CVariant::EType::FLOAT)
			{
				return var->getFloat() <= _variable.getFloat();
			}
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
	else if (opType == "lower")
	{
		_operation = EOperation::LOWER;
	}
	else if (opType == "lower_equal")
	{
		_operation = EOperation::LOWER_EQUAL;
	}
    else
    {
      _operation = EOperation::EQUAL;
    }
    return true;
  }


  bool MultipleVariableTransition::checkCondition(CContext& ctx) const
  {

    /* Only checks ands (to check ors, the solution is to make another transition */
    bool result = true;
    for (int i = 0; i < _variables.size() && result; i++) {
      const CVariant* var = ctx.getVariable(_variables[i].getName());
      if (var && var->getType() == _variables[i].getType())

        if (_operations[i] == EOperation::EQUAL) {
          if (var->getType() == CVariant::EType::BOOL)
          {
            result &= var->getBool() == _variables[i].getBool();
          }
          else if (var->getType() == CVariant::EType::INT)
          {
            result &= var->getInt() == _variables[i].getInt();
          }
          else if (var->getType() == CVariant::EType::FLOAT)
          {
            result &= var->getFloat() == _variables[i].getFloat();
          }
          else if (var->getType() == CVariant::EType::HANDLE)
          {
            result &= var->getHandle() == _variables[i].getHandle();
          }
          else if (var->getType() == CVariant::EType::STRING)
          {
            result &= var->getString() == _variables[i].getString();
          }
        }
        else if (_operations[i] == EOperation::GREATER) {
          if (var->getType() == CVariant::EType::INT)
          {
            result &= var->getInt() > _variables[i].getInt();
          }
          else if (var->getType() == CVariant::EType::FLOAT)
          {
            result &= var->getFloat() > _variables[i].getFloat();
          }
        }
        else if (_operations[i] == EOperation::GREATER_EQUAL) {
          if (var->getType() == CVariant::EType::INT)
          {
            result &= var->getInt() >= _variables[i].getInt();
          }
          else if (var->getType() == CVariant::EType::FLOAT)
          {
            result &= var->getFloat() >= _variables[i].getFloat();
          }
        }
        else if (_operations[i] == EOperation::LOWER) {
          if (var->getType() == CVariant::EType::INT)
          {
            result &= var->getInt() < _variables[i].getInt();
          }
          else if (var->getType() == CVariant::EType::FLOAT)
          {
            result &= var->getFloat() < _variables[i].getFloat();
          }
        }
        else if (_operations[i] == EOperation::LOWER_EQUAL) {
          if (var->getType() == CVariant::EType::INT)
          {
            result &= var->getInt() <= _variables[i].getInt();
          }
          else if (var->getType() == CVariant::EType::FLOAT)
          {
            result &= var->getFloat() <= _variables[i].getFloat();
          }
        }
    }
	  return result;
  }

  bool MultipleVariableTransition::load(const json& jData)
  {
    for (int i = 0; i < jData["variables"].size(); i++) {
      CVariant variable;
      EOperation operation = EOperation::EQUAL;
      variable.load(jData["variables"][i]);

      std::string opType = "equal";

      if (jData.count("operations")>= 1 && jData["operations"].size() > i) {
        opType = jData["operations"][i].value("operation", std::string("equal"));
      }

      if (opType == "greater")
      {
       operation = EOperation::GREATER;
      }
      else if (opType == "greater_equal")
      {
       operation = EOperation::GREATER_EQUAL;
      }
      else if (opType == "lower")
      {
        operation = EOperation::LOWER;
      }
      else if (opType == "lower_equal")
      {
        operation = EOperation::LOWER_EQUAL;
      }
      else
      {
       operation = EOperation::EQUAL;
      }

      _variables.push_back(variable);
      _operations.push_back(operation);
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

  bool AnimationTransition::checkCondition(CContext& ctx) const
  {

	  CEntity* e = ctx.getOwner();
	  TCompPlayerAnimator *compAnimator = e->get<TCompPlayerAnimator>();
	  return !compAnimator->isPlayingAnimation(compAnimator->getAnimationByName(anim_name));
  }

  bool AnimationTransition::load(const json& jData)
  {
	  anim_name = jData.value("animation_name", "");
	  return true;
  }
}
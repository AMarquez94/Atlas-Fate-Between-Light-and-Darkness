#pragma once

#include "transition.h"
#include "utils/variant.h"
#include "components\skeleton\comp_animator.h"
#include "components\player_controller\comp_player_animator.h"

namespace FSM
{
  class VariableTransition : public ITransition
  {
    virtual bool checkCondition(CContext& ctx) const override;
    virtual bool load(const json& jData) override;

  private:
    enum class EOperation { EQUAL = 0, GREATER, GREATER_EQUAL, LOWER, LOWER_EQUAL };
    CVariant _variable;
    EOperation _operation = EOperation::EQUAL;
  };

  class MultipleVariableTransition : public ITransition
  {
	  virtual bool checkCondition(CContext& ctx) const override;
	  virtual bool load(const json& jData) override;

  private:
	  enum class EOperation { EQUAL = 0, GREATER, GREATER_EQUAL, LOWER, LOWER_EQUAL };
	  std::vector<CVariant> _variables;
	  std::vector<EOperation> _operations;
  };

  class TimeTransition : public ITransition
  {
    virtual bool checkCondition(CContext& ctx) const override;
    virtual bool load(const json& jData) override;

  private:
    float _time = 0.f;
  };

  class AnimationTransition : public ITransition
  {
	  virtual bool checkCondition(CContext& ctx) const override;
	  virtual bool load(const json& jData) override;

  private:
	  std::string anim_name = "";
	  TCompAnimator::EAnimation animation;
	  bool firstFrame = true;
  };

}

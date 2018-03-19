#pragma once

#include <vector>

namespace FSM
{
  class CContext;
  class IState;

  class ITransition
  {
  public:
    virtual bool checkCondition(CContext& ctx) const { return false; }
    const IState* getSourceState() const { return _sourceState; }
    const IState* getTargetState() const { return _targetState; }
    bool isNegated() const { return _negated; }
    virtual bool load(const json& jData) { (void)jData; return false; }

    void setSource(const IState* state) { _sourceState = state; }
    void setTarget(const IState* state) { _targetState = state; }
    void setNegated(bool how) { _negated = how; }

  private:
    const IState* _sourceState = nullptr;
    const IState* _targetState = nullptr;
    bool _negated = false;
  };

  using VTransitions = std::vector<ITransition*>;
}

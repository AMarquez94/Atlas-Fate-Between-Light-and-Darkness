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
    virtual bool load(const json& jData) { (void)jData; return false; }

    void setSource(const IState* state) { _sourceState = state; }
    void setTarget(const IState* state) { _targetState = state; }

  private:
    const IState* _sourceState = nullptr;
    const IState* _targetState = nullptr;
  };

  using VTransitions = std::vector<ITransition*>;
}

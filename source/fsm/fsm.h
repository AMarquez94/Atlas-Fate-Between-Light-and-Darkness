#pragma once

#include "state.h"
#include "transition.h"
#include "utils/variant.h"
#include "resources/resource.h"

namespace FSM
{
  class CMachine : public IResource
  {
  public:

    bool load(const std::string& jData);

    const IState* getInitialState() const { return _initialState; }
    const VStates& getStates() const { return _states; }
    const VTransitions& getTransitions() const { return _transitions; }
    const MVariants& getVariables() const { return _variables; }

    const IState* getState(const std::string& name) const;
    void getStateTransitions(const IState* state, VTransitions& output) const;

  private:
    VStates _states;
    VTransitions _transitions;
    MVariants _variables;
    const IState* _initialState = nullptr;
  };
}

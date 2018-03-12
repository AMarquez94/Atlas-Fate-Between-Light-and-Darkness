#pragma once

namespace FSM
{
  class IState;
  class ITransition;

  class IStateInstancer
  {
  public:
    virtual IState* generate() = 0;
  };
  class ITransitionInstancer
  {
  public:
    virtual ITransition* generate() = 0;
  };

  class CFactory
  {
  public:
    void registerInstancer(const std::string& name, IStateInstancer* instancer);
    void registerInstancer(const std::string& name, ITransitionInstancer* instancer);
    IState* makeState(const std::string& stateType);
    ITransition* makeTransition(const std::string& stateType);

  private:
    std::map<std::string, IStateInstancer*> _stateInstancers;
    std::map<std::string, ITransitionInstancer*> _transitionInstancers;

    std::vector<IState*> _allStates;
    std::vector<ITransition*> _allTransitions;
  };

  // templatized instancers
  template<typename T>
  class StateInstancer : public IStateInstancer
  {
  public:
    IState * generate() override
    {
      return new T();
    }
  };

  template<typename T>
  class TransitionInstancer : public ITransitionInstancer
  {
  public:
    ITransition * generate() override
    {
      return new T();
    }
  };
}

#include "mcv_platform.h"
#include "factory.h"


//IStateInstancer* stInstancer = new StateInstancer<AnimationState>;
//stInstancer->generate();
//registerInstancer();

namespace FSM
{
  void CFactory::registerInstancer(const std::string& name, IStateInstancer* instancer)
  {
    _stateInstancers[name] = instancer;
  }

  void CFactory::registerInstancer(const std::string& name, ITransitionInstancer* instancer)
  {
    _transitionInstancers[name] = instancer;
  }

  IState* CFactory::makeState(const std::string& stateType)
  {
    IState* st = _stateInstancers[stateType]->generate();
    _allStates.push_back(st);
    return st;
  }

  ITransition* CFactory::makeTransition(const std::string& stateType)
  {
    ITransition* tr = _transitionInstancers[stateType]->generate();
    _allTransitions.push_back(tr);
    return tr;
  }
}
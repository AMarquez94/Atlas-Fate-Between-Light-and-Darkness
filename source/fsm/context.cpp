#include "mcv_platform.h"
#include "context.h"
#include "state.h"
#include "transition.h"
#include "fsm.h"

namespace FSM
{
  void CContext::setFSM(const CMachine* fsm)
  {
    _fsm = fsm;
  }

  void CContext::start()
  {
    if (_state == EState::IDLE)
    {
      _state = EState::RUNNING;
      setCurrentState(_fsm->getInitialState());
    }
  }

  void CContext::stop()
  {
    if (_state == EState::RUNNING)
    {
      if (_currentState)
      {
        _currentState->onFinish(*this);
        _currentState = nullptr;
      }
      _state = EState::IDLE;
    }
  }

  void CContext::update(float delta)
  {
    if (_state == EState::RUNNING)
    {
      bool finished = _currentState->update(delta, *this);
      if (finished)
      {
        _state = EState::FINISHED;
      }
      else
      {
        VTransitions stateTransitions;
        getStateTransitions(_currentState, stateTransitions);
        for (auto& transition : stateTransitions)
        {
          if (transition->checkCondition(*this))
          {
            setCurrentState(transition->getTargetState());
            break;
          }
        }
      }

    }
  }

  void CContext::setCurrentState(const IState* state)
  {
    // finish previous state
    if (_currentState)
    {
      _currentState->onFinish(*this);
      _currentState = nullptr;
    }

    // start new state
    if (state)
    {
      _currentState = state;
      _currentState->onStart(*this);
    }
  }

  const CVariant* CContext::getVariable(const std::string& name) const
  {
    return _variables.getVariant(name);
  }

  void CContext::getStateTransitions(const IState* state, VTransitions& output) const
  {
    for (auto& transition : _fsm->getTransitions())
    {
      if (transition->getSourceState() == state)
      {
        output.push_back(transition);
      }
    }
  }

}
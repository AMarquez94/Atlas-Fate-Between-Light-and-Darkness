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
        if (_fsm)
        {
            initializeVariables();
        }
    }

    void CContext::start()
    {
        if (_internalState == EState::IDLE)
        {
            _internalState = EState::RUNNING;
            setCurrentState(_fsm->getInitialState());
        }
    }

    void CContext::stop()
    {
        if (_internalState == EState::RUNNING)
        {
            if (_currentState)
            {
                _currentState->onFinish(*this);
                _currentState = nullptr;
            }
            _internalState = EState::IDLE;
        }
    }

    void CContext::update(float delta)
    {
        if (_internalState == EState::RUNNING)
        {
            _timeInState += delta;
            bool finished = _currentState->update(delta, *this);
            if (finished)
            {
                _internalState = EState::FINISHED;
            }
            else
            {
                VTransitions stateTransitions;
                _fsm->getStateTransitions(_currentState, stateTransitions);
                for (auto& transition : stateTransitions)
                {
                    bool pass = transition->checkCondition(*this);
                    if (transition->isNegated())
                    {
                        pass = !pass;
                    }
                    if (pass)
                    {
                        setCurrentState(transition->getTargetState());
                        break;
                    }
                }
            }
        }
    }

    void CContext::restart()
    {
        _internalState = EState::RUNNING;
        setCurrentState(_fsm->getInitialState());
        initializeVariables();
    }

    void CContext::setCurrentState(const IState* state)
    {
        // finish previous state
        if (_currentState)
        {
            _currentState->onFinish(*this);
            _currentState = nullptr;
        }
        _timeInState = 0.f;

        // start new state
        if (state)
        {
            _currentState = state;
            _currentState->onStart(*this);
        }
    }

    void CContext::setOwner(CHandle handle)
    {
        _owner = handle;
    }

    void CContext::setVariable(const std::string& name, bool value)
    {
        _variables.setVariant(name, value);
    }

    void CContext::setVariable(const std::string& name, int value)
    {
        _variables.setVariant(name, value);
    }

    void CContext::setVariable(const std::string& name, float value)
    {
        _variables.setVariant(name, value);
    }

    void CContext::setVariable(const std::string& name, CHandle value)
    {
        _variables.setVariant(name, value);
    }

    void CContext::setVariable(const std::string& name, const std::string& value)
    {
        _variables.setVariant(name, value);
    }

    void CContext::setVariable(const std::string& name, const CVariant& var)
    {
        _variables.setVariant(name, var);
    }

    const CVariant* CContext::getVariable(const std::string& name) const
    {
        return _variables.getVariant(name);
    }

    void CContext::initializeVariables()
    {
        _variables.clear();
        if (_fsm)
        {
            _variables = _fsm->getVariables();
        }
    }

}
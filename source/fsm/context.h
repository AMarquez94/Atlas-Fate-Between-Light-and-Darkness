#pragma once

#include "utils/variant.h"
#include "fsm.h"

namespace FSM
{
  class CMachine;
  class IState;

  class CContext
  {
  public:
    enum class EState { IDLE = 0, RUNNING, FINISHED };

    void setFSM(const CMachine* fsm);
    void start();
    void stop();
    void update(float delta);
    void restart();

    void setCurrentState(const IState* state);
    void setOwner(CHandle handle);
    
    void setVariable(const std::string& name, bool value);
    void setVariable(const std::string& name, int value);
    void setVariable(const std::string& name, float value);
    void setVariable(const std::string& name, CHandle value);
    void setVariable(const std::string& name, const std::string& value);
    void setVariable(const std::string& name, const CVariant& var);

    const CMachine* getFSM() const { return _fsm; }
    EState getInternalState() const { return _internalState; }
    const IState* getCurrentState() const { return _currentState; }
    CHandle getOwner() const { return _owner; }
    const MVariants& getVariables() const { return _variables; }
    const CVariant* getVariable(const std::string& name) const;
    float getTimeInState() const { return _timeInState; }

  private:
    void initializeVariables();

    const CMachine* _fsm = nullptr;
    EState _internalState = EState::IDLE;
    const IState* _currentState = nullptr;
    MVariants _variables;
    CHandle _owner;
    float _timeInState = 0.f;
  };
}

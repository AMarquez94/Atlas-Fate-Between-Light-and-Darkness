#pragma once

#include <vector>

namespace FSM
{
  class CContext;

  class IState
  {
  public:
    virtual void onStart(CContext& ctx) const {}
    virtual void onFinish(CContext& ctx) const {}
    virtual bool update(float dt, CContext& ctx) const { return _isFinal; }
    virtual bool load(const json& jData) { (void)jData; return false; }

    void setName(const std::string& name) { _name = name; }
    const std::string& getName() const { return _name; }
    bool isFinal() const { return _isFinal; }

  private:
    std::string _name;
    bool _isFinal = false;
  };

  using VStates = std::vector<IState*>;
}

#pragma once

class CGameState : public VModules
{
public:
  CGameState(const std::string& name)
    : _name(name)
  {}
  const std::string& getName() const { return _name; }
private:
  std::string _name;
};

using VGameStates = std::vector<CGameState*>;
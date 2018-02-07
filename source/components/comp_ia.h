#pragma once

#include "comp_base.h"

class IAIController;
 
class TCompIA : public TCompBase {
public:

  IAIController* ai_controller = nullptr;

  void debugInMenu();
  void update(float dt);
  void load(const json& j, TEntityParseContext& ctx);
};
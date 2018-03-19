#pragma once

#include "comp_base.h"
#include "fsm/context.h"


struct TMsgSetFSMVariable {
  CVariant variant;
  DECL_MSG_ID();
};

class TCompFSM : public TCompBase {
public:
  static void registerMsgs();
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  DECL_SIBLING_ACCESS();

  void onCreate(const TMsgEntityCreated& msg);
  void onVariableValue(const TMsgSetFSMVariable& msg);

private:
  FSM::CContext _context;
};
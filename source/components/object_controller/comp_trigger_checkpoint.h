#pragma once

#include "components/comp_base.h"

class TCompTriggerCheckpoint : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  static void registerMsgs();

private:
	bool used = false;
  void onMsgTriggerEnter(const TMsgTriggerEnter& msg);
};


#pragma once

#include "components/comp_base.h"

class TCompPlayerAttackCast : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  static void registerMsgs();

private:

  CHandle _hSource;

  physx::PxQueryFilterData PxPlayerAttackQueryFilterData;

  void onMsgEntityCreated(const TMsgEntityCreated& msg);
  void onMsgScenePaused(const TMsgScenePaused & msg);

  std::vector<CHandle> getEnemiesInRange();
};


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

  const std::vector<CHandle> getEnemiesInRange();

private:

  physx::PxQueryFilterData PxPlayerAttackQueryFilterData;
  physx::PxSphereGeometry geometry;

  void onMsgScenePaused(const TMsgScenePaused & msg);
};


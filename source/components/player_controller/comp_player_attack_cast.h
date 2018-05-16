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
  const std::vector<CHandle> getMovableObjectsInRange();
  const bool canAttackEnemiesInRange(CHandle& closestEnemyToAttack = CHandle());
  CHandle closestEnemyToMerge();
  CHandle closestObjectToMove();

private:

  physx::PxQueryFilterData PxPlayerAttackQueryFilterData;
  physx::PxQueryFilterData PxPlayerMovableObjectsQueryFilterData;

  physx::PxSphereGeometry geometry;
  float attack_fov, grabObject_fov;

  void onMsgScenePaused(const TMsgScenePaused & msg);
};


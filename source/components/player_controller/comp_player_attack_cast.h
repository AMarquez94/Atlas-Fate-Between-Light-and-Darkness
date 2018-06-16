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
  const bool canAttackEnemiesInRange(CHandle& closestEnemyToAttack = CHandle());
  CHandle closestEnemyToMerge();

  const std::vector<CHandle> getMovableObjectsInRange();
  CHandle getClosestMovableObjectInRange();

private:

  physx::PxQueryFilterData PxPlayerAttackQueryFilterData;
  physx::PxQueryFilterData PxPlayerMoveObjectsQueryFilterData;
  physx::PxSphereGeometry geometryAttack;
  physx::PxSphereGeometry geometryMoveObjects;
  float attack_fov;
  float moveObjects_fov;

  void onMsgScenePaused(const TMsgScenePaused & msg);
};


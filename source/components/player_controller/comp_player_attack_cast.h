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

  const std::vector<CHandle> getButtonsInRange();
  CHandle getClosestButtonInRange();

private:

  physx::PxQueryFilterData PxPlayerAttackQueryFilterData;
  physx::PxQueryFilterData PxPlayerMoveObjectsQueryFilterData;
  physx::PxQueryFilterData PxPlayerButtonInteractQueryFilterData;
  physx::PxSphereGeometry geometryAttack;
  physx::PxSphereGeometry geometryMoveObjects;
  physx::PxSphereGeometry geometryButtons;
  float attack_fov;
  float moveObjects_fov;
  float button_fov;

  void onMsgScenePaused(const TMsgScenePaused & msg);
};


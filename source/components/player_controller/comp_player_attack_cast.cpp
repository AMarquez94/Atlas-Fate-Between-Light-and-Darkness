#include "mcv_platform.h"
#include "comp_player_attack_cast.h"
#include "components/comp_transform.h"
#include "components/comp_hierarchy.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"
#include "components/ia/comp_bt_patrol.h"
#include "components/ia/comp_bt_mimetic.h"

DECL_OBJ_MANAGER("player_attack_cast", TCompPlayerAttackCast);

void TCompPlayerAttackCast::debugInMenu() {
}

void TCompPlayerAttackCast::load(const json& j, TEntityParseContext& ctx) {
  geometryAttack.radius = j.value("radiusAttack", 2.f);
  geometryMoveObjects.radius = j.value("radiusMoveObject", 0.35f);
  attack_fov = deg2rad(j.value("attack_fov", 120.f));
  moveObjects_fov = deg2rad(j.value("moveObjects_fov", 20.f));

  physx::PxFilterData pxFilterData;

  pxFilterData.word0 = FilterGroup::Enemy;
  PxPlayerAttackQueryFilterData.data = pxFilterData;
  PxPlayerAttackQueryFilterData.flags = physx::PxQueryFlag::eDYNAMIC;

  pxFilterData.word0 = FilterGroup::MovableObject;
  PxPlayerMoveObjectsQueryFilterData.data = pxFilterData;
  PxPlayerMoveObjectsQueryFilterData.flags = physx::PxQueryFlag::eDYNAMIC;
}

void TCompPlayerAttackCast::registerMsgs()
{
	DECL_MSG(TCompPlayerAttackCast, TMsgScenePaused, onMsgScenePaused);
}

void TCompPlayerAttackCast::onMsgScenePaused(const TMsgScenePaused & msg)
{
	paused = !paused;
}

const std::vector<CHandle> TCompPlayerAttackCast::getEnemiesInRange()
{
  std::vector<CHandle> enemies_in_range;

  TCompTransform* tPos = get<TCompTransform>();
  
  if (geometryAttack.isValid()) {

    std::vector<physx::PxOverlapHit> hits;
    if (EnginePhysics.Overlap(geometryAttack, tPos->getPosition(), hits, PxPlayerAttackQueryFilterData)) {
      for (int i = 0; i < hits.size(); i++) {
        CHandle hitCollider;
        hitCollider.fromVoidPtr(hits[i].actor->userData);
        if (hitCollider.isValid()) {
          CHandle enemy = hitCollider.getOwner();
          if (enemy.isValid()) {
            enemies_in_range.push_back(enemy);
          }
        }
      }
    }
  }

  return enemies_in_range;
}

const bool TCompPlayerAttackCast::canAttackEnemiesInRange(CHandle& closestEnemyToAttack)
{
  bool canAttackNow = false;
  const std::vector<CHandle> enemies = getEnemiesInRange();
  closestEnemyToAttack = CHandle();

  TCompTransform * mypos = get<TCompTransform>();

  for (int i = 0; i < enemies.size() && !canAttackNow; i++) {

    CEntity * enemy = enemies[i];
    TCompTransform *ePos = enemy->get<TCompTransform>();
    TCompTags * eTag = enemy->get<TCompTags>();

    if (eTag->hasTag(getID("patrol"))) {
      TCompAIPatrol * cPatrol = enemy->get<TCompAIPatrol>();
      if (mypos->isInHorizontalFov(ePos->getPosition(), attack_fov) &&
        !ePos->isInFront(mypos->getPosition()) &&
        !cPatrol->isStunned()) {
        canAttackNow = true;
        closestEnemyToAttack = enemies[i];
      }
    }
    else if (eTag->hasTag(getID("mimetic"))) {
      TCompAIMimetic * cMimetic = enemy->get<TCompAIMimetic>();
      if (mypos->isInHorizontalFov(ePos->getPosition(), attack_fov) && !cMimetic->isStunned()) {
        canAttackNow = true;
        closestEnemyToAttack = enemies[i];
      }
    }
  }
  return canAttackNow;
}

CHandle TCompPlayerAttackCast::closestEnemyToMerge()
{
    CHandle closestEnemy = CHandle();
    const std::vector<CHandle> enemies = getEnemiesInRange();
  
    TCompTransform * mypos = get<TCompTransform>();
  
    for (int i = 0; i < enemies.size() && !closestEnemy.isValid(); i++) {
  
      CEntity * enemy = enemies[i];
      TCompTransform *ePos = enemy->get<TCompTransform>();
      TCompTags * eTag = enemy->get<TCompTags>();
  
      if (eTag->hasTag(getID("patrol"))) {
        TCompAIPatrol * cPatrol = enemy->get<TCompAIPatrol>();
        if (mypos->isInHorizontalFov(ePos->getPosition(), attack_fov) && cPatrol->isStunned()) {
          closestEnemy = enemies[i];
        }
      }
    }
  
    return closestEnemy;
}

const std::vector<CHandle> TCompPlayerAttackCast::getMovableObjectsInRange()
{
  std::vector<CHandle> movable_objects_in_range;

  TCompTransform* tPos = get<TCompTransform>();

  if (geometryMoveObjects.isValid()) {

    std::vector<physx::PxOverlapHit> hits;
    if (EnginePhysics.Overlap(geometryMoveObjects, tPos->getPosition(), hits, PxPlayerMoveObjectsQueryFilterData)) {
      for (int i = 0; i < hits.size(); i++) {
        CHandle hitCollider;
        hitCollider.fromVoidPtr(hits[i].actor->userData);
        if (hitCollider.isValid()) {
          CHandle object = hitCollider.getOwner();
          if (object.isValid()) {
            movable_objects_in_range.push_back(object);
          }
        }
      }
    }
  }

  return movable_objects_in_range;
}

CHandle TCompPlayerAttackCast::getClosestMovableObjectInRange()
{
  bool found = false;
  CHandle closestMovableObject = CHandle();
  const std::vector<CHandle> movable_objects = getMovableObjectsInRange();

  TCompTransform * mypos = get<TCompTransform>();

  for (int i = 0; i < movable_objects.size() && !found; i++) {

    CEntity * movable_object = movable_objects[i];
    TCompTransform *ePos = movable_object->get<TCompTransform>();
    TCompRigidbody *eRigidbody = movable_object->get<TCompRigidbody>();

    if (mypos->isInHorizontalFov(ePos->getPosition(), moveObjects_fov) && eRigidbody->is_grounded) {
      found = true;
      closestMovableObject = movable_objects[i];
    }
  }
  return closestMovableObject;
}

void TCompPlayerAttackCast::update(float dt)
{}

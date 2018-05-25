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
  geometry.radius = j.value("radius", 2.f);
  attack_fov = deg2rad(j.value("attack_fov", 120.f));
  grabObject_fov = deg2rad(j.value("grabObject_fov", 40.f));
  grabEnemyHorizontal_fov = deg2rad(j.value("grabEnemyHorizontal_fov", 90.f));
  grabEnemyVertical_fov = deg2rad(j.value("grabEnemyVertical_fov", 90.f));


  physx::PxFilterData pxFilterData;
  pxFilterData.word0 = FilterGroup::Enemy;
  PxPlayerAttackQueryFilterData.data = pxFilterData;
  PxPlayerAttackQueryFilterData.flags = physx::PxQueryFlag::eDYNAMIC;

  //Movable objects in range
  physx::PxFilterData pxFilterData2;
  pxFilterData2.word0 = FilterGroup::Movable | FilterGroup::IgnoreMovable;
  PxPlayerMovableObjectsQueryFilterData.data = pxFilterData2;
  PxPlayerMovableObjectsQueryFilterData.flags = physx::PxQueryFlag::eDYNAMIC;

  //Already moving objects
  physx::PxFilterData pxCheckCollisionFilterData;
  pxCheckCollisionFilterData.word0 = FilterGroup::Movable | FilterGroup::Enemy | FilterGroup::Wall;
  PxMovingObjectQuery.data = pxCheckCollisionFilterData;
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
  
  if (geometry.isValid()) {

    std::vector<physx::PxOverlapHit> hits;
    if (EnginePhysics.SphereCast(geometry, tPos->getPosition(), hits, PxPlayerAttackQueryFilterData)) {
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

const std::vector<CHandle> TCompPlayerAttackCast::getMovableObjectsInRange()
{
	std::vector<CHandle> objects_in_range;

	TCompTransform* tPos = get<TCompTransform>();

	if (geometry.isValid()) {

		std::vector<physx::PxOverlapHit> hits;
		if (EnginePhysics.SphereCast(geometry, tPos->getPosition(), hits, PxPlayerMovableObjectsQueryFilterData)) {
			for (int i = 0; i < hits.size(); i++) {
				CHandle hitCollider;
				hitCollider.fromVoidPtr(hits[i].actor->userData);
				if (hitCollider.isValid()) {
					CHandle object = hitCollider.getOwner();
					if (object.isValid()) {
						objects_in_range.push_back(object);
					}
				}
			}
		}
	}

	return objects_in_range;
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
		bool inFov = mypos->isInFov(ePos->getPosition(), grabEnemyHorizontal_fov, grabEnemyVertical_fov);
        if (inFov && cPatrol->isStunned()) {
          closestEnemy = enemies[i];
        }
      }
    }
  
    return closestEnemy;
}

CHandle TCompPlayerAttackCast::closestObjectToMove()
{
	CHandle closestMovableObject = CHandle();
	const std::vector<CHandle> objects = getMovableObjectsInRange();

	TCompTransform * mypos = get<TCompTransform>();

	for (int i = 0; i < objects.size() && !closestMovableObject.isValid(); i++) {

		CEntity * object = objects[i];
		TCompTransform *ePos = object->get<TCompTransform>();
		TCompTags * eTag = object->get<TCompTags>();

		if (eTag->hasTag(getID("movable"))) {
			TCompTransform * movableObject = object->get<TCompTransform>();
			if (mypos->isInHorizontalFov(ePos->getPosition(), grabObject_fov)) {
				closestMovableObject = objects[i];
			}
		}
	}

	return closestMovableObject;
}

void TCompPlayerAttackCast::update(float dt)
{
}

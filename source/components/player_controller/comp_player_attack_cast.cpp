#include "mcv_platform.h"
#include "comp_player_attack_cast.h"
#include "components/comp_transform.h"
#include "components/comp_hierarchy.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("player_attack_cast", TCompPlayerAttackCast);

void TCompPlayerAttackCast::debugInMenu() {
}

void TCompPlayerAttackCast::load(const json& j, TEntityParseContext& ctx) {
  geometry.radius = j.value("radius", 2.f);

  physx::PxFilterData pxFilterData;
  pxFilterData.word0 = FilterGroup::Enemy;
  //pxFilterData.word1 = FilterGroup::Enemy;
  PxPlayerAttackQueryFilterData.data = pxFilterData;
  PxPlayerAttackQueryFilterData.flags = physx::PxQueryFlag::eDYNAMIC;
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

void TCompPlayerAttackCast::update(float dt)
{
}

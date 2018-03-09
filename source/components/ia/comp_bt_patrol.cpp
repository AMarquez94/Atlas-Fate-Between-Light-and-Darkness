#include "mcv_platform.h"
#include "comp_bt_patrol.h"
#include "btnode.h"
#include "components/comp_name.h"

DECL_OBJ_MANAGER("ai_patrol", TCompAIPatrol);

void TCompAIPatrol::debugInMenu() {

}


void TCompAIPatrol::load(const json& j, TEntityParseContext& ctx) {
	createRoot("patrol", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("patrol", "manageStun", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("manageStun", "shadowmerged", BTNode::EType::ACTION, (BTCondition)&TCompAIPatrol::conditionShadowMerged, (BTAction)&TCompAIPatrol::actionShadowMerged, nullptr);
	addChild("manageStun", "manageFixed", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionFixed, nullptr, nullptr);
	addChild("manageStun", "stunned", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionStunned, nullptr);
	addChild("manageFixed", "fixed", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionFixed, nullptr);
	addChild("manageFixed", "beginAlertFixed", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionBeginAlert, nullptr);
	addChild("manageFixed", "closestWptFixed", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);
	addChild("patrol", "endAlertRoot", BTNode::EType::ACTION, (BTCondition)&TCompAIPatrol::conditionEndAlert, (BTAction)&TCompAIPatrol::actionEndAlert, nullptr);
	addChild("patrol", "manageDoPatrol", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);

	addChild("manageDoPatrol", "managePlayerSeen", BTNode::EType::PRIORITY, (BTCondition)&TCompAIPatrol::conditionPlayerSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "managePlayerWasSeen", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionPlayerWasSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "managePatrolSeen", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionPatrolSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "managePatrolWasSeen", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionPatrolWasSeen, nullptr, nullptr);
	addChild("manageDoPatrol", "goToWpt", BTNode::EType::ACTION, (BTCondition)&TCompAIPatrol::conditionGoToWpt, (BTAction)&TCompAIPatrol::actionGoToWpt, (BTAssert)&TCompAIPatrol::assertGoToWpt);
	addChild("manageDoPatrol", "waitInWpt", BTNode::EType::ACTION, (BTCondition)&TCompAIPatrol::conditionWaitInWpt, (BTAction)&TCompAIPatrol::actionWaitInWpt, (BTAssert)&TCompAIPatrol::assertWaitInWpt);
	addChild("manageDoPatrol", "nextWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionNextWpt, nullptr);

	addChild("managePlayerSeen", "manageChase", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionChase, nullptr, nullptr);
	addChild("managePlayerSeen", "suspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionSuspect, nullptr);
	addChild("manageChase", "shootInhibitor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionShootInhibitor, nullptr);
	addChild("manageChase", "beginAlertChase", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionBeginAlert, nullptr);
	addChild("manageChase", "manageGoingToAttack", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
	addChild("manageGoingToAttack", "managePlayerAttacked", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPatrol::conditionPlayerAttacked, nullptr, nullptr);
	addChild("manageGoingToAttack", "ChasePlayer", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionChasePlayer, (BTAssert)&TCompAIPatrol::assertPlayerInFov);
	addChild("managePlayerAttacked", "attack", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionAttack, nullptr);
	addChild("managePlayerAttacked", "endAlertAttacked", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionEndAlert, nullptr);
	addChild("managePlayerAttacked", "closestWptAttacked", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);

	addChild("managePlayerWasSeen", "goToPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionGoToPlayerLastPos, (BTAssert)&TCompAIPatrol::assertPlayerNotInFov);
	addChild("managePlayerWasSeen", "lookForPlayerSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionLookForPlayer, (BTAssert)&TCompAIPatrol::assertPlayerNotInFov);
	addChild("managePlayerWasSeen", "closestWptPlayerSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);

	addChild("managePatrolSeen", "markPatrolAsSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionMarkPatrolAsSeen, nullptr);
	addChild("managePatrolSeen", "goToPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionGoToPatrol, (BTAssert)&TCompAIPatrol::assertPlayerNotInFovAndPatrolInFov);
	addChild("managePatrolSeen", "fixPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionFixPatrol, nullptr);
	addChild("managePatrolSeen", "beginAlertPatrolSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionBeginAlert, nullptr);
	addChild("managePatrolSeen", "closestWptPatrolSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);

	addChild("managePatrolWasSeen", "goToPatrolPrevPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionGoToPatrolPrevPos, (BTAssert)&TCompAIPatrol::assertPlayerAndPatrolNotInFov);
	addChild("managePatrolWasSeen", "lookForPlayerPatrolWasSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionLookForPlayer, (BTAssert)&TCompAIPatrol::assertPlayerAndPatrolNotInFov);
	addChild("managePatrolWasSeen", "closestWptPatrolWasSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPatrol::actionClosestWpt, nullptr);
}

void TCompAIPatrol::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
	TCompName *tName = get<TCompName>();
	name = tName->getName();
}

void TCompAIPatrol::registerMsgs()
{
	DECL_MSG(TCompAIPatrol, TMsgEntityCreated, onMsgEntityCreated);
}




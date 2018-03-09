#pragma once

#include "components/comp_base.h"
#include "comp_bt.h"

class TCompAIPatrol : public TCompIAController {

private:

	DECL_SIBLING_ACCESS();

	void onMsgEntityCreated(const TMsgEntityCreated& msg);

public:
	void load(const json& j, TEntityParseContext& ctx) override;
	void debugInMenu();

	BTNode::ERes actionShadowMerged(float dt);
	BTNode::ERes actionStunned(float dt);
	BTNode::ERes actionFixed(float dt);
	BTNode::ERes actionBeginAlert(float dt);
	BTNode::ERes actionClosestWpt(float dt);
	BTNode::ERes actionEndAlert(float dt);
	BTNode::ERes actionGoToWpt(float dt);
	BTNode::ERes actionWaitInWpt(float dt);
	BTNode::ERes actionNextWpt(float dt);
	BTNode::ERes actionSuspect(float dt);
	BTNode::ERes actionShootInhibitor(float dt);
	BTNode::ERes actionChasePlayer(float dt);
	BTNode::ERes actionAttack(float dt);
	BTNode::ERes actionGoToPlayerLastPos(float dt);
	BTNode::ERes actionLookForPlayer(float dt);
	BTNode::ERes actionMarkPatrolAsSeen(float dt);
	BTNode::ERes actionGoToPatrol(float dt);
	BTNode::ERes actionFixPatrol(float dt);
	BTNode::ERes actionGoToPatrolPrevPos(float dt);

	bool conditionShadowMerged(float dt);
	bool conditionFixed(float dt);
	bool conditionEndAlert(float dt);
	bool conditionPlayerSeen(float dt);
	bool conditionPlayerWasSeen(float dt);
	bool conditionPatrolSeen(float dt);
	bool conditionPatrolWasSeen(float dt);
	bool conditionGoToWpt(float dt);
	bool conditionWaitInWpt(float dt);
	bool conditionChase(float dt);
	bool conditionPlayerAttacked(float dt);

	bool assertGoToWpt(float dt);
	bool assertWaitInWpt(float dt);
	bool assertPlayerInFov(float dt);
	bool assertPlayerNotInFov(float dt);
	bool assertPlayerNotInFovAndPatrolInFov(float dt);
	bool assertPlayerAndPatrolNotInFov(float dt);

	static void registerMsgs();
};
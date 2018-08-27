#pragma once

#include "components/comp_base.h"
#include "comp_bt_enemy.h"
#include "modules/module_ia.h"

class TCompAIPlayer : public TCompIAController {

public:

    enum EState { TUT_CROUCH = 0, TUT_SM, TUT_INHIBITOR, CINEMATIC, NUM_STATES };

    bool enabledPlayerAI;

    void load(const json& j, TEntityParseContext& ctx) override;
    void preUpdate(float dt) override;
    void postUpdate(float dt) override;
    void debugInMenu();

    static void registerMsgs();

    BTNode::ERes actionGoToWpt(float dt);
    BTNode::ERes actionDefault(float dt);

    BTNode::ERes actionResetTimersCrouchTutorial(float dt);
    BTNode::ERes actionResetTimersSMTutorial(float dt);
    BTNode::ERes actionResetTimersRemoveInhibitorTutorial(float dt);
    BTNode::ERes actionResetBT(float dt);
    BTNode::ERes actionAnimationWalk(float dt);
    BTNode::ERes actionAnimationCrouch(float dt);
    BTNode::ERes actionAnimationIdle(float dt);
    BTNode::ERes actionStartSM(float dt);
    BTNode::ERes actionAnimationSM(float dt);
    BTNode::ERes actionEndSM (float dt);
    BTNode::ERes actionAnimationInhibitor(float dt);
    BTNode::ERes actionAnimationInhibitorMiddle(float dt);
    BTNode::ERes actionAnimationInhibitorFinnish(float dt);

    bool conditionHasBeenEnabled(float dt);
    bool conditionCinematicMode(float dt);
    bool conditionCrouchTutorial(float dt);
    bool conditionSMTutorial(float dt);
    bool conditionRemoveInhibitorTutorial(float dt);

    //Auxiliar
    bool move(float dt);

private:

	float _speed, _rotationSpeed;
	std::vector<Waypoint> _waypoints;
	int _currentWaypoint;
    float _timer = 0;
    float _maxTimer = 0;

    TCompAIPlayer::EState _currentState;

	void onMsgPlayerAIEnabled(const TMsgPlayerAIEnabled& msg);
	void onMsgEntityCreated(const TMsgEntityCreated& msg);

	//load
	void loadActions() override;
	void loadConditions() override;
	void loadAsserts() override;

    TCompAIPlayer::EState getStateEnumFromString(const std::string& stateName);

    DECL_SIBLING_ACCESS();
};
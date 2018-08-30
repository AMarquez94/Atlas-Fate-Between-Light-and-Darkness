#pragma once

#include "components/comp_base.h"
#include "comp_bt_enemy.h"
#include "modules/module_ia.h"

class TCompAIPlayer : public TCompIAController {

public:

    enum EState { TUT_CROUCH = 0, TUT_SM, TUT_INHIBITOR, TUT_ATTACK, TUT_SM_FALL, TUT_SM_VER, TUT_BOX, TUT_SONAR, TUT_BUTTON, TUT_SM_FENCE, CINEMATIC, NUM_STATES };

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
    BTNode::ERes actionResetTimersAttackTutorial(float dt);
    BTNode::ERes actionResetTimersSMFallTutorial(float dt);
    BTNode::ERes actionResetTimersSMVerTutorial(float dt);
    BTNode::ERes actionResetTimersBoxTutorial(float dt);
    BTNode::ERes actionResetTimersSonarTutorial(float dt);
    BTNode::ERes actionResetTimersButtonTutorial(float dt);
    BTNode::ERes actionResetTimersSMFenceTutorial(float dt);
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
    BTNode::ERes actionAnimationStandingCrouch(float dt);
    BTNode::ERes actionAnimationAttack(float dt);
    BTNode::ERes actionWaitAttack(float dt);
    BTNode::ERes actionWait(float dt);
    BTNode::ERes actionAnimationFalling(float dt);
    BTNode::ERes actionSMVerMove(float dt);
    BTNode::ERes actionSMVerMoveFront(float dt);
    BTNode::ERes actionSMVerTurn(float dt);
    BTNode::ERes actionAnimationWalkWithMovement(float dt);
    BTNode::ERes actionAnimationGrabBox(float dt);
    BTNode::ERes actionAnimationWalkBox(float dt);
    BTNode::ERes actionAnimationSonar(float dt);
    BTNode::ERes actionAnimationPressButton(float dt);

    bool conditionHasBeenEnabled(float dt);
    bool conditionCinematicMode(float dt);
    bool conditionCrouchTutorial(float dt);
    bool conditionSMTutorial(float dt);
    bool conditionRemoveInhibitorTutorial(float dt);
    bool conditionAttackTutorial(float dt);
    bool conditionSMFallTutorial(float dt);
    bool conditionSMVerTutorial(float dt);
    bool conditionButtonTutorial(float dt);
    bool conditionBoxTutorial(float dt);
    bool conditionSonarTutorial(float dt);
    bool conditionSMFenceTutorial(float dt);

    //Auxiliar
    bool move(float dt);

private:

	float _speed, _rotationSpeed;
	std::vector<Waypoint> _waypoints;
	int _currentWaypoint;
    float _timer = 0;
    float _maxTimer = 0;
    VEC3 initial_pos = VEC3::Zero;
    QUAT initial_rot = VEC4::Zero;

    TCompAIPlayer::EState _currentState;

	void onMsgPlayerAIEnabled(const TMsgPlayerAIEnabled& msg);
	void onMsgEntityCreated(const TMsgEntityCreated& msg);
	void onMsgEntityGroupCreated(const TMsgEntitiesGroupCreated& msg);

	//load
	void loadActions() override;
	void loadConditions() override;
	void loadAsserts() override;

    TCompAIPlayer::EState getStateEnumFromString(const std::string& stateName);
    CHandle h_sm_tutorial;

    DECL_SIBLING_ACCESS();
};
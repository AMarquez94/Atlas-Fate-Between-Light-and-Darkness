#pragma once

#include "components/comp_base.h"
#include "comp_bt_enemy.h"
#include "modules/module_ia.h"

class TCompAIPatrol : public TCompAIEnemy {

private:

    enum EState {
        CINEMATIC_DEAD = 0,
        CINEMATIC_INHIBITOR,
        NUM_STATES
    };

    /* Cinematic states */
    EState _currentCinematicState;
    bool _enabledCinematicAI = false;
    float _cinematicTimer = 0.f;
    float _cinematicMaxTime = 0.f;

    /* Atributes */
    float amountRotated = 0.f;
    VEC3 lastStunnedPatrolKnownPos = VEC3::Zero;
    bool startLightsOn = false;
    bool hasBeenShadowMerged = false;
    bool hasBeenFixed = false;
    bool disabledLanterns = false;
    std::vector<CHandle> ignoredPatrols;

    //bool is_tutorial = false;
    //std::string tutorial_name = "";
    //float timer = 0.f;
    //float maxTimer = 0.f;

    DECL_SIBLING_ACCESS();

    void onMsgEntityCreated(const TMsgEntityCreated& msg);
    void onMsgPlayerDead(const TMsgPlayerDead& msg);
    void onMsgPatrolStunned(const TMsgEnemyStunned& msg);
    void onMsgPatrolShadowMerged(const TMsgPatrolShadowMerged& msg);
    void onMsgPatrolFixed(const TMsgPatrolFixed& msg);
    void onMsgNoiseListened(const TMsgNoiseMade& msg);
    void onMsgLanternsDisable(const TMsgLanternsDisable& msg);
    void onMsgCinematicState(const TMsgCinematicState& msg);
	void onMsgAnimationCompleted(const TMsgAnimationCompleted& msg);
	void onMsgWarned(const TMsgWarnEnemy& msg);
	void onMsgResetPatrolLights(const TMsgResetPatrolLights& msg);

    /* Aux functions */
    void turnOnLight();
    void turnOffLight();
    bool isStunnedPatrolInFov(float fov, float maxChaseDistance);
    bool isStunnedPatrolInPos(VEC3 lastPos);
    CHandle getPatrolInPos(VEC3 lastPos);
    float getMaxChaseDistance();
    TCompAIPatrol::EState getStateEnumFromString(const std::string& stateName);
    void warnClosestPatrols();

	/* Completed Animations Checkers */

	bool inhibitorAnimationCompleted = false;
	bool attackAnimationCompleted = false;

    //load
    void loadActions() override;
    void loadConditions() override;
    void loadAsserts() override;

public:
    void preUpdate(float dt) override;
    void postUpdate(float dt) override;
    void load(const json& j, TEntityParseContext& ctx) override;
    void debugInMenu();

    BTNode::ERes actionShadowMerged(float dt);
    BTNode::ERes actionStunned(float dt);
    BTNode::ERes actionFixed(float dt);
    BTNode::ERes actionBeginAlert(float dt);
    BTNode::ERes actionClosestWpt(float dt);
    BTNode::ERes actionEndAlert(float dt);
    BTNode::ERes actionMarkNoiseAsInactive(float dt);
    BTNode::ERes actionGenerateNavmeshNoise(float dt);
    BTNode::ERes actionGoToNoiseSource(float dt);
    BTNode::ERes actionWaitInNoiseSource(float dt);
    BTNode::ERes actionGenerateNavmeshWpt(float dt);
    BTNode::ERes actionGoToWpt(float dt);
    BTNode::ERes actionWaitInWpt(float dt);
    BTNode::ERes actionNextWpt(float dt);
    BTNode::ERes actionSuspect(float dt);
    BTNode::ERes actionMarkPlayerAsSeen(float dt);
    BTNode::ERes actionShootInhibitor(float dt);
    BTNode::ERes actionGenerateNavmeshChase(float dt);
    BTNode::ERes actionWarnClosestDrone(float dt);
    BTNode::ERes actionRotateTowardsUnreachablePlayer(float dt);
    BTNode::ERes actionChasePlayer(float dt);
    BTNode::ERes actionAttack(float dt);
    BTNode::ERes actionRotateToNoiseSource(float dt);
    BTNode::ERes actionResetPlayerWasSeenVariables(float dt);
    BTNode::ERes actionGoToPlayerLastPos(float dt);
    BTNode::ERes actionLookForPlayer(float dt);
    BTNode::ERes actionGenerateNavmeshGoToPatrol(float dt);
    BTNode::ERes actionGoToPatrol(float dt);
    BTNode::ERes actionFixPatrol(float dt);
    BTNode::ERes actionMarkPatrolAsLost(float dt);

    BTNode::ERes actionDieAnimation(float dt);
    BTNode::ERes actionDeadAnimation(float dt);
    BTNode::ERes actionResetBT(float dt);
    BTNode::ERes actionResetInhibitorCinematicTimers(float dt);
    BTNode::ERes actionWait(float dt);
    BTNode::ERes actionAnimationShootInhibitor(float dt);

    bool conditionManageStun(float dt);
    bool conditionEndAlert(float dt);
    bool conditionShadowMerged(float dt);
    bool conditionFixed(float dt);
    bool conditionIsArtificialNoise(float dt);
    bool conditionIsNaturalNoise(float dt);
    bool conditionPlayerSeen(float dt);
    bool conditionPlayerWasSeen(float dt);
    bool conditionPatrolSeen(float dt);
    bool conditionFixPatrol(float dt);
    bool conditionGoToWpt(float dt);
    bool conditionWaitInWpt(float dt);
    bool conditionChase(float dt);
    bool conditionPlayerAttacked(float dt);
    bool conditionIsDestUnreachable(float dt);

    bool conditionIsCinematic(float dt);
    bool conditionDeadCinematic(float dt);
    bool conditionInhibitorCinematic(float dt);

    bool assertPlayerInFov(float dt);
    bool assertPlayerNotInFov(float dt);
    bool assertPlayerAndPatrolNotInFov(float dt);
    bool assertNotHeardArtificialNoise(float dt);
    bool assertNotPlayerInFovNorArtificialNoise(float dt);
    bool assertPlayerNotInFovNorNoise(float dt);
    bool assertPlayerAndPatrolNotInFovNotNoise(float dt);
    bool assertCantReachDest(float dt);
    bool assertCanReachDest(float dt);

    const std::string getStateForCheckpoint();

	static void registerMsgs();

    /* LUA functions */
    void launchInhibitor();

    void attackPlayer();
    void playStepParticle(bool left);
    void shakeCamera(float max_amount, float max_distance, float duration);
    void playSlamParticle();

	void playAnimationByName(const std::string & animationName) override;

	void resetAnimationCompletedBooleans();
};
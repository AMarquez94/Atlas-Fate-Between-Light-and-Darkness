#pragma once

#include "components/comp_base.h"
#include "comp_bt_enemy.h"
#include "modules/module_ia.h"

class TCompAIPatrol : public TCompAIEnemy {

private:

    /* Atributes */
    float amountRotated = 0.f;
    VEC3 lastStunnedPatrolKnownPos = VEC3::Zero;
    bool startLightsOn = false;
    bool hasBeenShadowMerged = false;
    bool hasBeenFixed = false;
    bool disabledLanterns = false;

    DECL_SIBLING_ACCESS();

    void onMsgEntityCreated(const TMsgEntityCreated& msg);
    void onMsgPlayerDead(const TMsgPlayerDead& msg);
    void onMsgPatrolStunned(const TMsgEnemyStunned& msg);
    void onMsgPatrolShadowMerged(const TMsgPatrolShadowMerged& msg);
    void onMsgPatrolFixed(const TMsgPatrolFixed& msg);
    void onMsgNoiseListened(const TMsgNoiseMade& msg);
    void onMsgLanternsDisable(const TMsgLanternsDisable& msg);

    /* Aux functions */
    void turnOnLight();
    void turnOffLight();
    bool isStunnedPatrolInFov(float fov, float maxChaseDistance);
    bool isStunnedPatrolInPos(VEC3 lastPos);
    CHandle getPatrolInPos(VEC3 lastPos);

    //load
    void loadActions() override;
    void loadConditions() override;
    void loadAsserts() override;

public:
    void preUpdate(float dt) override;
    void postUpdate(float dt) override;
    void load(const json& j, TEntityParseContext& ctx) override;
    void debugInMenu();

    //Auxiliar functions
    bool getStartLightsOn() { return startLightsOn; }

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

    bool assertPlayerInFov(float dt);
    bool assertPlayerNotInFov(float dt);
    bool assertPlayerAndPatrolNotInFov(float dt);
    bool assertNotHeardArtificialNoise(float dt);
    bool assertNotPlayerInFovNorArtificialNoise(float dt);
    bool assertPlayerNotInFovNorNoise(float dt);
    bool assertPlayerAndPatrolNotInFovNotNoise(float dt);

    const std::string getStateForCheckpoint();

	static void registerMsgs();

	void playAnimationByName(const std::string & animationName) override;
};
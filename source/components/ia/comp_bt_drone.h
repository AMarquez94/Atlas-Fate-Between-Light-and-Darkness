#pragma once

#include "components/comp_base.h"
#include "comp_bt_enemy.h"
#include "modules/module_ia.h"

class TCompAIDrone : public TCompAIEnemy {

private:

    /* Atributes */

    float speed = 3.f;
    float rotationSpeedChaseDeg = 359.f;
    float rotationSpeedChase;
    std::string entityToChase = "The Player";
    float fovDeg = 120.f;
    float fov;
    float autoChaseDistance = 10.f;
    float maxChaseDistance = 35.f;
    float maxTimeSuspecting = 3.f;
    float dcrSuspectO_Meter = .3f;
    float incrBaseSuspectO_Meter = .5f;
    float distToAttack = 1.5f;

    /* Timers */
    float timerWaitingInObservation = 0.f;
    float timerToExplode = 0.f;

    bool isSlept = false;
    bool isActive = false;
    bool goingInactive = false;
    VEC3 initialPos;
    VEC3 initialLookAt;
    float rotationSpeedObservation = deg2rad(40.f);
    float rotationSpeedPatrolling = deg2rad(80.f);
    float waitTimeInLasPlayerPos = 3.f;
    float chaseSpeed = 6.f;

    float amountRotatedObserving = 0.f;
    float maxAmountRotateObserving = deg2rad(45.f);

    float rotationSpeedNoise = deg2rad(120.f);

    VEC3 pushedDirection = VEC3::Zero;
    float pushedTime = 0.f;

    DECL_SIBLING_ACCESS();

    void onMsgEntityCreated(const TMsgEntityCreated& msg);
    void onMsgPlayerDead(const TMsgPlayerDead& msg);
    void onMsgMimeticStunned(const TMsgEnemyStunned& msg);
    void onMsgNoiseListened(const TMsgNoiseMade& msg);
    void onMsgPhysxContact(const TMsgPhysxContact& msg);

    /* Aux functions */
    void setGravityToFaceWall();

    //load
    void loadActions() override;
    void loadConditions() override;
    void loadAsserts() override;

public:
    void preUpdate(float dt) override;
    void postUpdate(float dt) override;
    void load(const json& j, TEntityParseContext& ctx) override;
    void debugInMenu();

    /* ACTIONS */
    BTNode::ERes actionFall(float dt);
    BTNode::ERes actionExplode(float dt);
    BTNode::ERes actionEndAlert(float dt);
    BTNode::ERes actionMarkPlayerAsSeen(float dt);
    BTNode::ERes actionGenerateNavmeshChase(float dt);
    BTNode::ERes actionChaseAndShoot(float dt);
    BTNode::ERes actionMarkNoiseAsInactive(float dt);
    BTNode::ERes actionGenerateNavmeshArtificialNoise(float dt);
    BTNode::ERes actionGoToNoiseSource(float dt);
    BTNode::ERes actionWaitInNoiseSource(float dt);
    BTNode::ERes actionClosestWpt(float dt);
    BTNode::ERes actionMarkOrderAsReceived(float dt);
    BTNode::ERes actionGenerateNavmeshOrder(float dt);
    BTNode::ERes actionGoToOrderPos(float dt);
    BTNode::ERes actionWaitInOrderPos(float dt);
    BTNode::ERes actionGenerateNavmeshPlayerLost(float dt);
    BTNode::ERes actionGoToPlayerLastPos(float dt);
    BTNode::ERes actionWaitInPlayerLastPos(float dt);
    BTNode::ERes actionGenerateNavmeshSuspect(float dt);
    BTNode::ERes actionSuspect(float dt);
    BTNode::ERes actionRotateToNoiseSource(float dt);
    BTNode::ERes actionGenerateNavmeshWpt(float dt);
    BTNode::ERes actionGoToWpt(float dt);
    BTNode::ERes actionResetTimer(float dt);
    BTNode::ERes actionWaitInWpt(float dt);

    /* CONDITIONS */
    bool conditionHasBeenStunned(float dt);
    bool conditionIsPlayerDead(float dt);
    bool conditionIsPlayerSeenForSure(float dt);
    bool conditionHasHeardArtificialNoise(float dt);
    bool conditionHasReceivedEnemyOrder(float dt);
    bool conditionPlayerHasBeenLost(float dt);
    bool conditionIsPlayerInFov(float dt);
    bool conditionHasHeardNaturalNoise(float dt);

    /* ASSERTS */
    bool assertNotPlayerInFovForSure(float dt);
    bool assertNotPlayerInFovNorArtificialNoise(float dt);
    bool assertNotHeardArtificialNoise(float dt);
    bool assertNotPlayerInFovNorNoise(float dt);

    const std::string getStateForCheckpoint();

    static void registerMsgs();

    void playAnimationByName(const std::string & animationName) override;

};
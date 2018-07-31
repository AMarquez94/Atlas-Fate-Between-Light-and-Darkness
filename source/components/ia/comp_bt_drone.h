#pragma once

#include "components/comp_base.h"
#include "comp_bt_enemy.h"
#include "modules/module_ia.h"

class TCompAIDrone : public TCompAIEnemy {

private:

    /* Atributes */

    float maxSpeed = 2.f;
    float maxChaseSpeed = 5.f;
    float upSpeed = 4.f;
    VEC3 upOffset = VEC3(0, 5.f, 0.f);
    float lerpValue = 0.06f;
    VEC3 prevDirection = VEC3::Zero;
    VEC3 currentDirection = VEC3::Zero;
    float flyingUpOffset = 1.f;   //Offset for up/down flight
    float flyingDownOffset = 2.f;   //Offset for up/down flight
    float addedOffset = 0.5f;
    physx::PxBoxGeometry geometrySweep;

    VEC3 lastPlayerKnownPositionOffset = VEC3::Zero;

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
    float rotationSpeedObservation = deg2rad(30.f);
    float rotationSpeedPatrolling = deg2rad(80.f);
    float waitTimeInLasPlayerPos = 3.f;
    float chaseSpeed = 3.5f;

    float amountRotatedObserving = 0.f;
    float maxAmountRotateObserving = deg2rad(45.f);

    float rotationSpeedNoise = deg2rad(120.f);

    /* Order */
    bool hasReceivedOrder = false;
    CHandle hOrderSource = CHandle();
    VEC3 orderPosition = VEC3::Zero;
    float timerWaitingInOrderPos = 0;
    float maxTimeWaitingInOrderPos = 4;
    std::chrono::steady_clock::time_point lastTimeOrderReceived;


    /* Lantern */
    CHandle hLantern;
    CHandle hLanternLight;
    bool lanternPatrollingLeft = false;
    float startingPitch = 0;
    float rotationSpeedLantern = deg2rad(20.f);

    /* Lantern rotation lerping */
    QUAT lerpingStartingRotation = VEC3::Zero;
    float timeToLerpLanternRot = 1.f;
    float timeLerpingLanternRot = 0.f;

    DECL_SIBLING_ACCESS();

    void onMsgEntityCreated(const TMsgEntityCreated& msg);
    void onMsgPlayerDead(const TMsgPlayerDead& msg);
    void onMsgDroneStunned(const TMsgEnemyStunned& msg);
    void onMsgNoiseListened(const TMsgNoiseMade& msg);
    void onMsgOrderReceived(const TMsgOrderReceived& msg);

    //load
    void loadActions() override;
    void loadConditions() override;
    void loadAsserts() override;

    /* AUX */
    bool moveToDestDrone(VEC3 dest, float speed, float dt, bool alsoHeight = true);
    void waitInPosDrone(VEC3 dest, float dt, float speed, float rotSpeed, VEC3 lookAt = VEC3::Zero);
    void moveLanternPatrolling(VEC3 objective, float dt, bool resetPitch = true);
    bool isEntityInFovDrone(const std::string& entityToChase);
    void stabilizeRotations(float dt);
    bool isRespectingVerticalOffset(VEC3 position, float dt);
    bool isRespectingHorizontalOffset(VEC3 position, float dt);

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
    BTNode::ERes actionNextWpt(float dt);

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
    bool assertNotPlayerInFovForSureNorOrder(float dt);
    bool assertNotPlayerInFovNorArtificialNoiseNorOrder(float dt);
    bool assertNotPlayerInFovNorNoiseNorOrder(float dt);
    bool assertNotPlayerInFovNorOrder(float dt);
    bool assertNotPlayerInFovForSureNorNextToNoise(float dt);


    const std::string getStateForCheckpoint();

    static void registerMsgs();

    void playAnimationByName(const std::string & animationName) override;
};
#include "mcv_platform.h"
#include "btnode.h"
#include "components/comp_name.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/object_controller/comp_cone_of_light.h"
#include "geometry/angular.h"
#include "comp_bt_drone.h"
#include "components/physics/comp_rigidbody.h"
#include "components/physics/comp_collider.h"
#include "components/lighting/comp_emission_controller.h"
#include "physics/physics_collider.h"
#include "render/render_utils.h"
#include "components/ia/comp_mimetic_animator.h"
#include "render/render_objects.h"
#include "components/object_controller/comp_noise_emitter.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("ai_drone", TCompAIDrone);

void TCompAIDrone::debugInMenu() {

    TCompAIEnemy::debugInMenu();
}

void TCompAIDrone::preUpdate(float dt)
{
    TCompTransform* myPos = get<TCompTransform>();
    myPos->setPosition(myPos->getPosition() + pushedDirection * 0.1f);
    /* TODO: Avoid hardcodeadas */
    pushedDirection = VEC3::Lerp(pushedDirection, VEC3::Zero, Clamp(pushedTime * 2.f, 0.f, 1.f));
    pushedTime = pushedTime * 2.f + dt;
}

void TCompAIDrone::postUpdate(float dt)
{

}

void TCompAIDrone::load(const json& j, TEntityParseContext& ctx) {

    loadActions();
    loadConditions();
    loadAsserts();
    //TCompIAController::loadTree(j);

    createRoot("mimetic", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
    addChild("mimetic", "manageStun", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasBeenStunned, nullptr, nullptr);
    addChild("manageStun", "stunned", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionStunned, nullptr);
    addChild("manageStun", "exploded", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionExplode, nullptr);

    addChild("mimetic", "manageInactive", BTNode::EType::PRIORITY, (BTCondition)&TCompAIDrone::conditionIsNotActive, nullptr, nullptr);
    addChild("manageInactive", "manageInactiveTypeSleep", BTNode::EType::PRIORITY, (BTCondition)&TCompAIDrone::conditionIsSlept, nullptr, nullptr);
    addChild("manageInactive", "manageInactiveTypeWall", BTNode::EType::PRIORITY, (BTCondition)&TCompAIDrone::conditionIsTypeWall, nullptr, nullptr);
    addChild("manageInactive", "manageInactiveTypeFloor", BTNode::EType::PRIORITY, (BTCondition)&TCompAIDrone::conditionIsTypeFloor, nullptr, nullptr);

    addChild("manageInactiveTypeSleep", "sleep", BTNode::EType::ACTION, (BTCondition)&TCompAIDrone::conditionNotListenedNoise, (BTAction)&TCompAIDrone::actionSleep, nullptr);
    addChild("manageInactiveTypeSleep", "wakeUp", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWakeUp, nullptr);

    addChild("manageInactiveTypeWall", "manageObserveTypeWall", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionIsNotPlayerInFovAndNotNoise, nullptr, nullptr);
    addChild("manageInactiveTypeWall", "setActiveTypewall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSetActive, nullptr);

    addChild("manageObserveTypeWall", "resetVariablesObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionResetObserveVariables, nullptr);
    addChild("manageObserveTypeWall", "turnLeftObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionObserveLeft, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageObserveTypeWall", "waitLeftObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitObserving, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageObserveTypeWall", "turnRightObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionObserveRight, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageObserveTypeWall", "waitRightObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitObserving, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);

    addChild("manageInactiveTypeFloor", "manageInactiveBehaviour", BTNode::EType::PRIORITY, (BTCondition)&TCompAIDrone::conditionIsNotPlayerInFovAndNotNoise, nullptr, nullptr);
    addChild("manageInactiveTypeFloor", "setActiveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSetActive, nullptr);
    addChild("manageInactiveBehaviour", "manageObserveTypeFloor", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasNotWaypoints, nullptr, nullptr);
    addChild("manageInactiveBehaviour", "managePatrol", BTNode::EType::SEQUENCE, nullptr, nullptr, nullptr);

    addChild("manageObserveTypeFloor", "resetVariablesObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionResetObserveVariables, nullptr);
    addChild("manageObserveTypeFloor", "turnLeftObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionObserveLeft, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageObserveTypeFloor", "waitLeftObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitObserving, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageObserveTypeFloor", "turnRightObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionObserveRight, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageObserveTypeFloor", "waitRightObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitObserving, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);

    addChild("managePatrol", "generateNavmeshGoToWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshWpt, nullptr);
    addChild("managePatrol", "goToWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToWpt, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("managePatrol", "resetTimerWaitInWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionResetTimerWaiting, nullptr);
    addChild("managePatrol", "waitInWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInWpt, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("managePatrol", "nextWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionNextWpt, nullptr);

    addChild("mimetic", "manageChase", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionPlayerSeenForSure, nullptr, nullptr);
    addChild("manageChase", "jumpFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionJumpFloor, nullptr);
    addChild("manageChase", "resetVariablesChase", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionResetVariablesChase, nullptr);
    addChild("manageChase", "chasePlayerWithNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionChasePlayerWithNoise, nullptr);

    addChild("mimetic", "manageArtificialNoise", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasHeardArtificialNoise, nullptr, nullptr);
    addChild("manageArtificialNoise", "markArtificialNoiseAsInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionMarkNoiseAsInactive, nullptr);
    addChild("manageArtificialNoise", "jumpFloorNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionJumpFloor, nullptr);
    addChild("manageArtificialNoise", "generateNavmeshNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshNoiseSource, nullptr/*(BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise*/);
    addChild("manageArtificialNoise", "goToArtificialNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToNoiseSource, nullptr/*(BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise*/);
    addChild("manageArtificialNoise", "waitInArtificialNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInNoiseSource, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorArtificialNoise);
    addChild("manageArtificialNoise", "setGoInactiveArtificialNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSetGoInactive, nullptr/*(BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise*/);

    addChild("mimetic", "manageSuspect", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionNotSurePlayerInFov, nullptr, nullptr);
    addChild("manageSuspect", "suspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSuspect, nullptr);
    addChild("manageSuspect", "rotateToInitialPosSuspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionRotateToInitialPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);

    addChild("mimetic", "manageNaturalNoise", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasHeardNaturalNoise, nullptr, nullptr);
    addChild("manageNaturalNoise", "markNaturalNoiseAsInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionMarkNoiseAsInactive, nullptr);
    addChild("manageNaturalNoise", "rotateToNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionRotateToNoiseSource, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageNaturalNoise", "setGoInactiveAfterNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSetGoInactive, nullptr);

    addChild("mimetic", "managePlayerLost", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionNotGoingInactive, nullptr, nullptr);
    addChild("managePlayerLost", "generateNavmeshPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshPlayerLastPos, nullptr);
    addChild("managePlayerLost", "goToPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToPlayerLastPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("managePlayerLost", "resetTimerWaitInPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionResetTimerWaiting, nullptr);
    addChild("managePlayerLost", "waitInPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInPlayerLastPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("managePlayerLost", "setGoInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSetGoInactive, nullptr);

    addChild("mimetic", "manageGoingInactive", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
    addChild("manageGoingInactive", "manageGoingInactiveTypeWall", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionIsTypeWall, nullptr, nullptr);
    addChild("manageGoingInactiveTypeWall", "generateNavmeshInitialPosTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshInitialPos, nullptr);
    addChild("manageGoingInactiveTypeWall", "goToInitialPosTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToInitialPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageGoingInactiveTypeWall", "rotateToInitialPosTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionRotateToInitialPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageGoingInactiveTypeWall", "jumpWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionJumpWall, nullptr);
    addChild("manageGoingInactiveTypeWall", "holdOnWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionHoldOnWall, nullptr);
    addChild("manageGoingInactiveTypeWall", "setInactiveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSetInactive, nullptr);
    addChild("manageGoingInactive", "manageGoingInactiveTypeWpts", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasWpts, nullptr, nullptr);
    addChild("manageGoingInactiveTypeWpts", "closestWptTypeWpts", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionClosestWpt, nullptr);
    addChild("manageGoingInactiveTypeWpts", "setInactiveTypeWpts", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSetInactive, nullptr);
    addChild("manageGoingInactive", "manageGoingInactiveTypeFloor", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionIsTypeFloor, nullptr, nullptr);
    addChild("manageGoingInactiveTypeFloor", "generateNavmeshInitialPosTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshInitialPos, nullptr);
    addChild("manageGoingInactiveTypeFloor", "goToInitialPosTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToInitialPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageGoingInactiveTypeFloor", "rotateToInitialPosTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionRotateToInitialPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("manageGoingInactiveTypeFloor", "setInactiveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSetInactive, nullptr);

    if (j.count("waypoints") > 0) {
        auto& j_waypoints = j["waypoints"];
        _waypoints.clear();
        for (auto it = j_waypoints.begin(); it != j_waypoints.end(); ++it) {

            Waypoint wpt;
            assert(it.value().count("position") == 1);
            assert(it.value().count("lookAt") == 1);

            wpt.position = loadVEC3(it.value()["position"]);
            wpt.lookAt = loadVEC3(it.value()["lookAt"]);
            wpt.minTime = it.value().value("minTime", 5.f);
            addWaypoint(wpt);
        }
    }

    type = parseStringMimeticType(j.value("type", ""));
    rotationSpeedChase = deg2rad(rotationSpeedChaseDeg);
    fov = deg2rad(fovDeg);
    currentWaypoint = 0;

    enemyColor.colorNormal = j.count("colorNormal") ? loadVEC4(j["colorNormal"]) : VEC4(1, 1, 1, 1);
    enemyColor.colorSuspect = j.count("colorSuspect") ? loadVEC4(j["colorSuspect"]) : VEC4(1, 1, 0, 1);
    enemyColor.colorAlert = j.count("colorAlert") ? loadVEC4(j["colorAlert"]) : VEC4(1, 0, 0, 1);
    enemyColor.colorDead = j.count("colorDead") ? loadVEC4(j["colorDead"]) : VEC4(0, 0, 0, 0);

    btType = BTType::MIMETIC;
}

void TCompAIDrone::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
    TCompName *tName = get<TCompName>();
    name = tName->getName();

    TCompTransform *tTransform = get<TCompTransform>();
    initialLookAt = tTransform->getFront();
    initialPos = tTransform->getPosition();

    myHandle = CHandle(this);

}

void TCompAIDrone::onMsgPlayerDead(const TMsgPlayerDead& msg) {

    alarmEnded = false;
}

void TCompAIDrone::onMsgMimeticStunned(const TMsgEnemyStunned & msg)
{
    hasBeenStunned = true;

    TCompEmissionController *eController = get<TCompEmissionController>();
    eController->blend(enemyColor.colorDead, 0.1f);
    TCompTransform *mypos = get<TCompTransform>();
    float y, p, r;
    mypos->getYawPitchRoll(&y, &p, &r);
    p = p + deg2rad(89.f);
    mypos->setYawPitchRoll(y, p, r);

    TCompGroup* cGroup = get<TCompGroup>();
    CEntity* eCone = cGroup->getHandleByName("Cone of Vision");
    TCompRender * coneRender = eCone->get<TCompRender>();
    coneRender->visible = false;

    lastPlayerKnownPos = VEC3::Zero;

    current = nullptr;
}

void TCompAIDrone::onMsgNoiseListened(const TMsgNoiseMade & msg)
{
    /* TODO: code in parent */
    bool isManagingArtificialNoise = isNodeSonOf(current, "goToArtificialNoiseSource");
    bool isWaitingInNoiseSource = isNodeName(current, "waitInArtificialNoiseSource");
    bool isManagingNaturalNoise = isNodeSonOf(current, "manageNaturalNoise");
    bool isChasingPlayer = isNodeSonOf(current, "manageChase");

    std::chrono::steady_clock::time_point newNoiseTime = std::chrono::steady_clock::now();

    if (!isChasingPlayer && (!isManagingArtificialNoise || isWaitingInNoiseSource)) {
        if (!isEntityInFov(entityToChase, fov - deg2rad(1.f), autoChaseDistance - 1.f)) {
            if (msg.isArtificialNoise) {
                hasHeardArtificialNoise = true;
            }
            else if (!isManagingNaturalNoise && !msg.isArtificialNoise) {
                hasHeardNaturalNoise = true;
            }
        }
    }

    /* Noise management */
    if (!hNoiseSource.isValid() || hNoiseSource == msg.hNoiseSource || isManagingNaturalNoise && msg.isArtificialNoise || std::chrono::duration_cast<std::chrono::seconds>(newNoiseTime - lastTimeNoiseWasHeard).count() > 1.5f) {

        /* Different noise sources (different enemies) => only hear if 1.5 seconds (hardcoded (TODO: Change)) passed || Same noise source => update noise settings */
        lastTimeNoiseWasHeard = newNoiseTime;
        noiseSourceChanged = noiseSource != msg.noiseOrigin;
        noiseSource = msg.noiseOrigin;
        hNoiseSource = msg.hNoiseSource;
    }
}

void TCompAIDrone::onMsgPhysxContact(const TMsgPhysxContact & msg)
{
    CEntity* other = msg.other_entity;
    TCompTags * otherTags = other->get <TCompTags>();
    if (otherTags && otherTags->hasTag(getID("patrol"))) {

        TCompTransform * otherTransform = other->get<TCompTransform>();
        TCompTransform * myTransform = get<TCompTransform>();

        if (VEC3::Distance(myTransform->getPosition(), otherTransform->getPosition()) < 2) {
            VEC3 direction = myTransform->getPosition() - otherTransform->getPosition();
            direction.Normalize();

            if (fabsf(direction.Dot(otherTransform->getFront())) > 0.8f) {
                if (otherTransform->isInLeft(myTransform->getPosition())) {
                    direction = direction + myTransform->getLeft();
                    direction.Normalize();
                }
            }
            pushedDirection = direction;
            pushedTime = 0.f;
        }
    }
}


/* TODO: REVISAR MUY MUCHO */
const std::string TCompAIDrone::getStateForCheckpoint()
{
    if (current) {
        std::string currName = current->getName();
        if (isNodeSonOf(current, "manageInactiveTypeWall")) {
            return "resetVariables";
        }
        else if (isNodeSonOf(current, "manageInactiveTypeFloor")) {
            if (isNodeSonOf(current, "manageObserveTypeFloor")) {
                return "resetVariables";
            }
            else {
                return "generateNavmeshGoToWpt";
            }
        }
        else {
            return "nextWpt";
        }
    }
    else {
        /* TODO: Gestionar tambien SLEEP */
        return "nextWpt";
    }
}

void TCompAIDrone::registerMsgs()
{
    DECL_MSG(TCompAIDrone, TMsgScenePaused, onMsgScenePaused);
    DECL_MSG(TCompAIDrone, TMsgAIPaused, onMsgAIPaused);
    DECL_MSG(TCompAIDrone, TMsgEntityCreated, onMsgEntityCreated);
    DECL_MSG(TCompAIDrone, TMsgPlayerDead, onMsgPlayerDead);
    DECL_MSG(TCompAIDrone, TMsgEnemyStunned, onMsgMimeticStunned);
    DECL_MSG(TCompAIDrone, TMsgNoiseMade, onMsgNoiseListened);
    DECL_MSG(TCompAIDrone, TMsgPhysxContact, onMsgPhysxContact);
}

void TCompAIDrone::loadActions() {
    actions_initializer.clear();

}

void TCompAIDrone::loadConditions() {
    conditions_initializer.clear();

}

void TCompAIDrone::loadAsserts() {
    asserts_initializer.clear();

}

/* ACTIONS */

BTNode::ERes TCompAIDrone::actionStunned(float dt)
{
    timerToExplode += dt;
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE);

    if (timerToExplode > 1.f) {
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIDrone::actionExplode(float dt)
{
    CHandle(this).getOwner().destroy();
    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionResetObserveVariables(float dt)
{
    timerWaitingInObservation = 0.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionObserveLeft(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::TURN_LEFT);
    if (isEntityInFov(entityToChase, fov, maxChaseDistance)) {
        return BTNode::ERes::LEAVE;
    }
    else {
        VEC3 objective = rotateVectorAroundAxis(initialLookAt, VEC3(0, 1.f, 0), maxAmountRotateObserving);
        TCompTransform *mypos = get<TCompTransform>();
        if (rotateTowardsVec(mypos->getPosition() + objective, rotationSpeedObservation, dt)) {
            return BTNode::ERes::LEAVE;
        }
        else {
            return BTNode::ERes::STAY;
        }
    }
}

BTNode::ERes TCompAIDrone::actionObserveRight(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::TURN_RIGHT);
    if (isEntityInFov(entityToChase, fov, maxChaseDistance)) {
        return BTNode::ERes::LEAVE;
    }
    else {
        VEC3 objective = rotateVectorAroundAxis(initialLookAt, VEC3(0, 1, 0), -maxAmountRotateObserving);
        TCompTransform *mypos = get<TCompTransform>();
        if (rotateTowardsVec(mypos->getPosition() + objective, rotationSpeedObservation, dt)) {
            return BTNode::ERes::LEAVE;
        }
        else {
            return BTNode::ERes::STAY;
        }
    }
}

BTNode::ERes TCompAIDrone::actionWaitObserving(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE);
    timerWaitingInObservation += dt;

    if (timerWaitingInObservation < 2.f) {
        return BTNode::ERes::STAY;
    }
    else {
        timerWaitingInObservation = 0.f;
        return BTNode::ERes::LEAVE;
    }
}

BTNode::ERes TCompAIDrone::actionSetActive(float dt)
{
    isActive = true;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionJumpFloor(float dt)
{
    TCompRigidbody *tCollider = get<TCompRigidbody>();

    tCollider->setNormalGravity(VEC3(0, -9.8f, 0));
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshWpt(float dt)
{
    TCompTransform *tTransform = get<TCompTransform>();
    generateNavmesh(tTransform->getPosition(), _waypoints[currentWaypoint].position);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToWpt(float dt)
{

    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::WALK);
    return moveToPoint(speed, rotationSpeedPatrolling, getWaypoint().position, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionResetTimerWaiting(float dt)
{
    timerWaitingInWpt = 0.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionWaitInWpt(float dt)
{

    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE);
    if (timerWaitingInWpt >= getWaypoint().minTime) {
        return BTNode::ERes::LEAVE;
    }
    else {
        timerWaitingInWpt += dt;
        TCompTransform *mypos = get<TCompTransform>();
        rotateTowardsVec(mypos->getPosition() + getWaypoint().lookAt, rotationSpeedObservation, dt);
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIDrone::actionNextWpt(float dt)
{
    timerWaitingInWpt = 0.f;
    currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionSleep(float dt)
{
    return BTNode::STAY;
}

BTNode::ERes TCompAIDrone::actionWakeUp(float dt)
{
    isSlept = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionSuspect(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::SUSPECTING);
    TCompEmissionController *eController = get<TCompEmissionController>();
    eController->blend(enemyColor.colorSuspect, 0.1f);
    // chase
    TCompTransform *mypos = get<TCompTransform>();
    CEntity *player = getEntityByName(entityToChase);
    TCompTransform *ppos = player->get<TCompTransform>();

    /* Distance to player */
    float distanceToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());

    if (distanceToPlayer <= autoChaseDistance && isEntityInFov(entityToChase, fov, maxChaseDistance)) {
        eController->blend(enemyColor.colorAlert, 0.1f);
        rotateTowardsVec(ppos->getPosition(), rotationSpeedObservation, dt);
        suspectO_Meter = 1.f;
    }
    else if (distanceToPlayer <= maxChaseDistance && isEntityInFov(entityToChase, fov, maxChaseDistance)) {
        suspectO_Meter = Clamp(suspectO_Meter + dt * incrBaseSuspectO_Meter, 0.f, 1.f);							//TODO: increment more depending distance and noise
        rotateTowardsVec(ppos->getPosition(), rotationSpeedObservation, dt);
    }
    else {
        suspectO_Meter = Clamp(suspectO_Meter - dt * dcrSuspectO_Meter, 0.f, 1.f);
    }

    if (suspectO_Meter <= 0.f || suspectO_Meter >= 1.f) {
        if (suspectO_Meter <= 0) {
            isActive = false;
            eController->blend(enemyColor.colorNormal, 0.1f);
        }
        else {
            eController->blend(enemyColor.colorAlert, 0.1f);
        }
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIDrone::actionRotateToNoiseSource(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::WALK);
    TCompTransform *myPos = get<TCompTransform>();
    bool isInObjective = rotateTowardsVec(noiseSource, rotationSpeedNoise, dt);
    return isInObjective ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshPlayerLastPos(float dt)
{
    TCompTransform *tpos = get<TCompTransform>();
    generateNavmesh(tpos->getPosition(), lastPlayerKnownPos);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionResetVariablesChase(float dt)
{
    goingInactive = false;
    isActive = true;
    hasHeardArtificialNoise = false;
    hasHeardNaturalNoise = false;

    /* Noise emitter */
    TCompNoiseEmitter * noiseEmitter = get<TCompNoiseEmitter>();
    noiseEmitter->makeNoise(20.f, .4f, true, false, true);

    TCompTransform *tpos = get<TCompTransform>();
    CEntity *player = getEntityByName(entityToChase);
    TCompTransform *ppos = player->get<TCompTransform>();

    generateNavmesh(tpos->getPosition(), ppos->getPosition());

    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionChasePlayerWithNoise(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::RUN);

    TCompTransform *mypos = get<TCompTransform>();
    CEntity *player = getEntityByName(entityToChase);
    TCompTransform *ppos = player->get<TCompTransform>();

    hasHeardArtificialNoise = false;
    hasHeardNaturalNoise = false;

    TCompEmissionController *eController = get<TCompEmissionController>();
    eController->blend(enemyColor.colorAlert, 0.1f);

    if (lastPlayerKnownPos != VEC3::Zero) {

        /* If we had the player's previous position, know where he is going */
        isLastPlayerKnownDirLeft = mypos->isInLeft(ppos->getPosition() - lastPlayerKnownPos);
    }

    if (lastPlayerKnownPos != ppos->getPosition()) {
        generateNavmesh(mypos->getPosition(), ppos->getPosition());
    }

    lastPlayerKnownPos = ppos->getPosition();

    float distToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
    if (!isEntityInFov(entityToChase, fov, maxChaseDistance) || distToPlayer >= maxChaseDistance + 0.5f) {
        TCompEmissionController *eController = get<TCompEmissionController>();
        eController->blend(enemyColor.colorSuspect, 0.1f);

        /* Cancel noise emitter */
        TCompNoiseEmitter * noiseEmitter = get<TCompNoiseEmitter>();
        noiseEmitter->makeNoise(-1.f, 10.f, false, false, true);

        return BTNode::ERes::LEAVE;
    }
    else if (distToPlayer <= 1.3f) {

        /* TODO: This fix is temporary */
        rotateTowardsVec(ppos->getPosition(), rotationSpeedChase, dt);
        return BTNode::ERes::STAY;

    }
    else {

        VEC3 nextPos = navmeshPath.size() > 0 && navmeshPathPoint < navmeshPath.size() ?
            navmeshPath[navmeshPathPoint] :
            ppos->getPosition();

        rotateTowardsVec(nextPos, rotationSpeedChase, dt);
        VEC3 vp = mypos->getPosition();
        VEC3 vfwd = mypos->getFront();
        vfwd.Normalize();
        vp = vp + chaseSpeed * dt * vfwd;
        mypos->setPosition(vp);

        if (VEC3::Distance2D(nextPos, vp) <= maxDistanceToNavmeshPoint) {
            navmeshPathPoint++;
        }
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIDrone::actionMarkNoiseAsInactive(float dt)
{
    timerWaitingInNoise = 0.f;
    hasHeardArtificialNoise = false;
    hasHeardNaturalNoise = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshNoiseSource(float dt)
{
    TCompTransform *tpos = get<TCompTransform>();
    noiseSourceChanged = false;
    generateNavmesh(tpos->getPosition(), noiseSource);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToNoiseSource(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::RUN);

    CEntity * ePlayer = getEntityByName(entityToChase);
    TCompTransform * ppos = get<TCompTransform>();
    VEC3 pp = ppos->getPosition();

    if (noiseSourceChanged) {
        generateNavmesh(pp, noiseSource);
        noiseSourceChanged = false;
    }

    if (isEntityInFov(entityToChase, fov - deg2rad(1.f), autoChaseDistance - 1.f)) {
        current = nullptr;
        return BTNode::ERes::LEAVE;
    }

    if (moveToPoint(speed, rotationSpeedPatrolling, noiseSource, dt)) {
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIDrone::actionWaitInNoiseSource(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE);
    TCompTransform *mypos = get<TCompTransform>();
    VEC3 vp = mypos->getPosition();
    CEntity * ePlayer = getEntityByName(entityToChase);
    TCompTransform * ppos = get<TCompTransform>();
    VEC3 pp = ppos->getPosition();

    if (entityToChase, fov - deg2rad(1.f), autoChaseDistance - 1.f) {
        return BTNode::ERes::LEAVE;
    }

    timerWaitingInNoise += dt;
    if (timerWaitingInNoise > 1.f) {
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIDrone::actionGoToPlayerLastPos(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::RUN);
    if (moveToPoint(speed, rotationSpeedChase, lastPlayerKnownPos, dt)) {
        lastPlayerKnownPos = VEC3::Zero;
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIDrone::actionWaitInPlayerLastPos(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE);
    timerWaitingInWpt += dt;
    if (timerWaitingInWpt < waitTimeInLasPlayerPos) {
        return BTNode::STAY;
    }
    else {
        return BTNode::ERes::LEAVE;
    }
}

BTNode::ERes TCompAIDrone::actionSetGoInactive(float dt)
{
    TCompEmissionController *eController = get<TCompEmissionController>();
    eController->blend(enemyColor.colorNormal, 0.1f);
    goingInactive = true;
    suspectO_Meter = 0.f;
    lastPlayerKnownPos = VEC3::Zero;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshInitialPos(float dt)
{
    TCompTransform *tTransform = get<TCompTransform>();
    generateNavmesh(tTransform->getPosition(), initialPos);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToInitialPos(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::WALK);
    TCompTransform *mypos = get<TCompTransform>();
    VEC3 initialPosWithMyY = VEC3(initialPos.x, mypos->getPosition().y, initialPos.z);

    return moveToPoint(speed, rotationSpeedPatrolling, initialPosWithMyY, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionRotateToInitialPos(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::WALK);
    TCompTransform *myPos = get<TCompTransform>();
    bool isInObjective = rotateTowardsVec(myPos->getPosition() + initialLookAt, rotationSpeedObservation, dt);
    return isInObjective ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionJumpWall(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::JUMP_TO_WALL);
    TCompRigidbody *tCollider = get<TCompRigidbody>();
    tCollider->Jump(VEC3(0, 25.f, 0));
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionHoldOnWall(float dt)
{

    TCompTransform * tTransform = get<TCompTransform>();
    if (tTransform->getPosition().y >= initialPos.y) {
        tTransform->setPosition(initialPos);
        setGravityToFaceWall();
        return BTNode::ERes::LEAVE;
    }
    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionSetInactive(float dt)
{
    isActive = false;
    goingInactive = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionClosestWpt(float dt)
{
    getClosestWpt();
    return BTNode::ERes::LEAVE;
}


/* CONDITIONS */

bool TCompAIDrone::conditionHasBeenStunned(float dt)
{
    return hasBeenStunned;
}

bool TCompAIDrone::conditionIsNotPlayerInFovAndNotNoise(float dt)
{
    return !hasHeardNaturalNoise && !hasHeardArtificialNoise && !isEntityInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIDrone::conditionIsNotActive(float dt)
{
    return !isActive;
}

bool TCompAIDrone::conditionHasWpts(float dt)
{
    return _waypoints.size() > 0;
}

bool TCompAIDrone::conditionIsSlept(float dt)
{
    return isSlept;
}

bool TCompAIDrone::conditionHasNotWaypoints(float dt)
{
    return _waypoints.size() == 0;
}

bool TCompAIDrone::conditionNotListenedNoise(float dt)
{
    return hasHeardNaturalNoise || hasHeardArtificialNoise;
}

bool TCompAIDrone::conditionNotSurePlayerInFov(float dt)
{
    return suspectO_Meter < 1.f && (suspectO_Meter > 0.f || isEntityInFov(entityToChase, fov, maxChaseDistance));
}

bool TCompAIDrone::conditionHasHeardNaturalNoise(float dt)
{
    return hasHeardNaturalNoise;
}

bool TCompAIDrone::conditionPlayerSeenForSure(float dt)
{
    return isEntityInFov(entityToChase, fov, maxChaseDistance) && suspectO_Meter >= 1.f;
}

bool TCompAIDrone::conditionHasHeardArtificialNoise(float dt)
{
    return hasHeardArtificialNoise;
}

bool TCompAIDrone::conditionIsPlayerInFov(float dt)
{
    return isEntityInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIDrone::conditionNotGoingInactive(float dt)
{
    return !goingInactive;
}

/* ASSERTS */

bool TCompAIDrone::assertNotPlayerInFovNorNoise(float dt)
{
    return !hasHeardArtificialNoise && !hasHeardNaturalNoise && !isEntityInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIDrone::assertNotPlayerInFov(float dt)
{
    return !isEntityInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIDrone::assertNotPlayerInFovNorArtificialNoise(float dt)
{
    return !hasHeardArtificialNoise && !isEntityInFov(entityToChase, fov, maxChaseDistance);
}

/* AUX FUNCTIONS */

void TCompAIDrone::setGravityToFaceWall()
{
    TCompRigidbody * tCollider = get<TCompRigidbody>();
    TCompTransform * tTransform = get<TCompTransform>();
    physx::PxRaycastHit hit;

    VEC3 directions[4] = { VEC3(1,0,0), VEC3(-1,0,0), VEC3(0,0,1), VEC3(0,0,-1) };
    float minDistance = INFINITY;
    VEC3 finalDir;

    for (int i = 0; i < 4; i++) {


        if (EnginePhysics.Raycast(tTransform->getPosition(), directions[i], 2.f, hit, physx::PxQueryFlag::eSTATIC)) {
            if (hit.distance < minDistance) {
                minDistance = hit.distance;
                finalDir = directions[i];
            }
        }
    }

    tCollider->setNormalGravity(finalDir * 9.8f);
}

void TCompAIDrone::playAnimationByName(const std::string & animationName)
{
    TCompMimeticAnimator * myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimationConverted(myAnimator->getAnimationByName(animationName));
}
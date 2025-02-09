#include "mcv_platform.h"
#include "comp_bt_mimetic.h"
#include "btnode.h"
#include "components/comp_name.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/object_controller/comp_laser.h"
#include "components/object_controller/comp_cone_of_light.h"
#include "geometry/angular.h"
#include "components/physics/comp_rigidbody.h"
#include "components/physics/comp_collider.h"
#include "components/lighting/comp_emission_controller.h"
#include "physics/physics_collider.h"
#include "render/render_utils.h"
#include "components/ia/comp_mimetic_animator.h"
#include "render/render_objects.h"
#include "components/object_controller/comp_noise_emitter.h"
#include "components/comp_tags.h"


DECL_OBJ_MANAGER("ai_mimetic", TCompAIMimetic);

void TCompAIMimetic::debugInMenu() {

    TCompAIEnemy::debugInMenu();
}

TCompAIMimetic::~TCompAIMimetic()
{
    //TCompAIEnemy::~TCompAIEnemy();
}

void TCompAIMimetic::preUpdate(float dt)
{
    TCompTransform* myPos = get<TCompTransform>();
    myPos->setPosition(myPos->getPosition() + pushedDirection * 0.1f);
    /* TODO: Avoid hardcodeadas */
    pushedDirection = VEC3::Lerp(pushedDirection, VEC3::Zero, Clamp(pushedTime * 2.f, 0.f, 1.f));
    pushedTime = pushedTime * 2.f + dt;
}

void TCompAIMimetic::postUpdate(float dt)
{
    if (current != nullptr && current->getName() == "chasePlayerWithNoise") {
        if (alarm.isValid() && alarm.isPlaying()) {
            float volume = alarm.getVolume();
            if (volume < 1.f) {
                volume = Clamp(volume + dt, 0.f, 1.f);
                alarm.setVolume(volume);
            }
        }
        else {
            TCompAudio* my_audio = get<TCompAudio>();
            alarm.stop();
            alarm = my_audio->playEvent("event:/Sounds/Enemies/Mimetic/MimeticAlarm");
        }
    }
    else {
        if (alarm.isValid() && alarm.isPlaying()) {
            float volume = alarm.getVolume();
            if (volume > 0.f) {
                volume = Clamp(volume - dt, 0.f, 1.f);
                alarm.setVolume(volume);
            }
            else {
                alarm.stop();
            }
        }
    }
}

void TCompAIMimetic::load(const json& j, TEntityParseContext& ctx) {

    loadActions();
    loadConditions();
    loadAsserts();
    //TCompIAController::loadTree(j);

    createRoot("mimetic", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
    addChild("mimetic", "manageStun", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionHasBeenStunned, nullptr, nullptr);
    addChild("manageStun", "stunned", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionStunned, nullptr);
    addChild("manageStun", "exploded", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionExplode, nullptr);

    addChild("mimetic", "manageInactive", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsNotActive, nullptr, nullptr);
    addChild("manageInactive", "manageInactiveTypeSleep", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsSlept, nullptr, nullptr);
    addChild("manageInactive", "manageInactiveTypeWall", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsTypeWall, nullptr, nullptr);
    addChild("manageInactive", "manageInactiveTypeFloor", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsTypeFloor, nullptr, nullptr);

    addChild("manageInactiveTypeSleep", "sleep", BTNode::EType::ACTION, (BTCondition)&TCompAIMimetic::conditionNotListenedNoise, (BTAction)&TCompAIMimetic::actionSleep, nullptr);
    addChild("manageInactiveTypeSleep", "wakeUp", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWakeUp, nullptr);

    addChild("manageInactiveTypeWall", "manageObserveTypeWall", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionIsNotPlayerInRaycastAndNotNoise, nullptr, nullptr);
    addChild("manageInactiveTypeWall", "setActiveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetActiveOnWall, nullptr);

    addChild("manageObserveTypeWall", "resetVariablesObserveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetObserveVariables, nullptr);
    addChild("manageObserveTypeWall", "waitTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitOnWall, (BTAssert)&TCompAIMimetic::assertNotPlayerInRaycastNorNoise);

    addChild("manageInactiveTypeFloor", "manageInactiveBehaviour", BTNode::EType::PRIORITY, (BTCondition)&TCompAIMimetic::conditionIsNotPlayerInFovAndNotNoise, nullptr, nullptr);
    addChild("manageInactiveTypeFloor", "setActiveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetActive, nullptr);
    addChild("manageInactiveBehaviour", "manageObserveTypeFloor", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionHasNotWaypoints, nullptr, nullptr);
    addChild("manageInactiveBehaviour", "managePatrol", BTNode::EType::SEQUENCE, nullptr, nullptr, nullptr);

    addChild("manageObserveTypeFloor", "resetVariablesObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetObserveVariables, nullptr);
    addChild("manageObserveTypeFloor", "turnLeftObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionObserveLeft, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("manageObserveTypeFloor", "waitLeftObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitObserving, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("manageObserveTypeFloor", "turnRightObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionObserveRight, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("manageObserveTypeFloor", "waitRightObserveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitObserving, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);

    addChild("managePatrol", "generateNavmeshGoToWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGenerateNavmeshWpt, nullptr);
    addChild("managePatrol", "goToWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToWpt, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("managePatrol", "resetTimerWaitInWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetTimerWaiting, nullptr);
    addChild("managePatrol", "waitInWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitInWpt, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("managePatrol", "nextWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionNextWpt, nullptr);

    addChild("mimetic", "manageChase", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionPlayerSeenForSure, nullptr, nullptr);
    addChild("manageChase", "jumpFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionJumpFloor, nullptr);
    addChild("manageChase", "resetVariablesChase", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetVariablesChase, nullptr);
    addChild("manageChase", "manageChasePlayer", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);

    addChild("manageChasePlayer", "manageUnreachableChase", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionIsDestUnreachable, nullptr, nullptr);
    addChild("manageUnreachableChase", "rotateToUnreachablePlayer", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionRotateTowardsPlayerWithNoise, (BTAssert)&TCompAIMimetic::assertCantReachDest);
    addChild("manageChasePlayer", "manageReachableChase", BTNode::EType::SEQUENCE, nullptr, nullptr, nullptr);
    addChild("manageReachableChase", "chasePlayerWithNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionChasePlayerWithNoise, (BTAssert)&TCompAIMimetic::assertCanReachDest);

    addChild("mimetic", "manageArtificialNoise", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionHasHeardArtificialNoise, nullptr, nullptr);
    addChild("manageArtificialNoise", "markArtificialNoiseAsInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionMarkNoiseAsInactive, nullptr);
    addChild("manageArtificialNoise", "jumpFloorNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionJumpFloor, nullptr);
    addChild("manageArtificialNoise", "generateNavmeshNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGenerateNavmeshNoiseSource, nullptr/*(BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise*/);
    addChild("manageArtificialNoise", "goToArtificialNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToNoiseSource, nullptr/*(BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise*/);
    addChild("manageArtificialNoise", "waitInArtificialNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitInNoiseSource, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorArtificialNoise);
    addChild("manageArtificialNoise", "setGoInactiveArtificialNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetGoInactive, nullptr/*(BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise*/);

    addChild("mimetic", "manageSuspect", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionNotSurePlayerInFov, nullptr, nullptr);
    addChild("manageSuspect", "suspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSuspect, nullptr);
    addChild("manageSuspect", "rotateToInitialPosSuspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionRotateToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);

    addChild("mimetic", "manageNaturalNoise", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionHasHeardNaturalNoise, nullptr, nullptr);
    addChild("manageNaturalNoise", "markNaturalNoiseAsInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionMarkNoiseAsInactive, nullptr);
    addChild("manageNaturalNoise", "rotateToNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionRotateToNoiseSource, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("manageNaturalNoise", "setGoInactiveAfterNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetGoInactive, nullptr);

    addChild("mimetic", "managePlayerLost", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionNotGoingInactive, nullptr, nullptr);
    addChild("managePlayerLost", "generateNavmeshPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGenerateNavmeshPlayerLastPos, nullptr);
    addChild("managePlayerLost", "goToPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToPlayerLastPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("managePlayerLost", "resetTimerWaitInPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionResetTimerWaiting, nullptr);
    addChild("managePlayerLost", "waitInPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionWaitInPlayerLastPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("managePlayerLost", "setGoInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetGoInactive, nullptr);

    addChild("mimetic", "manageGoingInactive", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
    addChild("manageGoingInactive", "manageGoingInactiveTypeWall", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionIsTypeWall, nullptr, nullptr);
    addChild("manageGoingInactiveTypeWall", "generateNavmeshInitialPosTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGenerateNavmeshInitialPos, nullptr);
    addChild("manageGoingInactiveTypeWall", "goToInitialPosTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("manageGoingInactiveTypeWall", "rotateToInitialPosTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionRotateToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("manageGoingInactiveTypeWall", "jumpWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionJumpWall, nullptr);
    addChild("manageGoingInactiveTypeWall", "holdOnWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionHoldOnWall, nullptr);
    addChild("manageGoingInactiveTypeWall", "setInactiveTypeWall", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetInactive, nullptr);
    addChild("manageGoingInactive", "manageGoingInactiveTypeWpts", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionHasWpts, nullptr, nullptr);
    addChild("manageGoingInactiveTypeWpts", "closestWptTypeWpts", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionClosestWpt, nullptr);
    addChild("manageGoingInactiveTypeWpts", "setInactiveTypeWpts", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetInactive, nullptr);
    addChild("manageGoingInactive", "manageGoingInactiveTypeFloor", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIMimetic::conditionIsTypeFloor, nullptr, nullptr);
    addChild("manageGoingInactiveTypeFloor", "generateNavmeshInitialPosTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGenerateNavmeshInitialPos, nullptr);
    addChild("manageGoingInactiveTypeFloor", "goToInitialPosTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionGoToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("manageGoingInactiveTypeFloor", "rotateToInitialPosTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionRotateToInitialPos, (BTAssert)&TCompAIMimetic::assertNotPlayerInFovNorNoise);
    addChild("manageGoingInactiveTypeFloor", "setInactiveTypeFloor", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIMimetic::actionSetInactive, nullptr);

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

    onCreate();
    btType = BTType::MIMETIC;
}

void TCompAIMimetic::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
    TCompName *tName = get<TCompName>();
    name = tName->getName();

    TCompTransform *tTransform = get<TCompTransform>();
    initialLookAt = tTransform->getFront();
    initialPos = tTransform->getPosition();

    if (type == EType::WALL) {
        setGravityToFaceWall();
    }

    myHandle = CHandle(this);

}

void TCompAIMimetic::onMsgPlayerDead(const TMsgPlayerDead& msg) {
    TCompEmissionController* my_emission = get<TCompEmissionController>();
    my_emission->blend(enemyColor.colorNormal, 0.5f);
    goingInactive = true;
    lastPlayerKnownPos = VEC3::Zero;
    alarmEnded = false;
    hasHeardArtificialNoise = false;
    hasHeardNaturalNoise = false;
    /* Cancel noise emitter */
    TCompNoiseEmitter * noiseEmitter = get<TCompNoiseEmitter>();
    noiseEmitter->makeNoise(-1.f, 10.f, false, false, true);
    setCurrent(nullptr);
}

void TCompAIMimetic::onMsgMimeticStunned(const TMsgEnemyStunned & msg)
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

void TCompAIMimetic::onMsgNoiseListened(const TMsgNoiseMade & msg)
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

void TCompAIMimetic::onMsgPhysxContact(const TMsgPhysxContact & msg)
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

void TCompAIMimetic::onMsgAnimationCompleted(const TMsgAnimationCompleted& msg) {

	if (msg.animation_name.compare("wakeup_jump") == 0) {
		wakeUpJumpAnimationCompleted = true;
	}
	if (msg.animation_name.compare("wakeup") == 0) {
		wakeUpAnimationCompleted = true;
	}
	if (msg.animation_name.compare("rest_jump") == 0) {
		restJumpAnimationCompleted = true;
	}
	if (msg.animation_name.compare("rest_wall") == 0) {
		restAnimationCompleted = true;
	}

}

void TCompAIMimetic::onMsgEnemyNothingHere(const TMsgEnemyNothingHere & msg)
{
    if (navmeshPath.size() > 0) {
        if (VEC3::Distance(navmeshPath[navmeshPath.size() - 1], msg.position) < 3.f) {
            if (isNodeSonOf(current, "manageChase") || isNodeSonOf(current, "manageChasePlayer")) {
                TCompEmissionController* my_emission = get<TCompEmissionController>();
                my_emission->blend(enemyColor.colorNormal, 0.5f);
                goingInactive = true;
                lastPlayerKnownPos = VEC3::Zero;
                setCurrent(nullptr);
            }
            else if (isNodeSonOf(current, "manageArtificialNoise")) {
                TCompEmissionController* my_emission = get<TCompEmissionController>();
                my_emission->blend(enemyColor.colorNormal, 0.5f);
                goingInactive = true;
                noiseSource = VEC3::Zero;
                hasHeardArtificialNoise = false;
                hasHeardNaturalNoise = false;
                setCurrent(nullptr);
            }
        }
    }
}


/* TODO: REVISAR MUY MUCHO */
const std::string TCompAIMimetic::getStateForCheckpoint()
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
            if (type == EType::FLOOR) {
                if (_waypoints.size() > 0) {
                    return "closestWptTypeWpts";
                }
                else {
                    return "generateNavmeshInitialPosTypeFloor";
                }
            }
            else if (type == EType::WALL) {
                TCompTransform * tPos = get<TCompTransform>();
                if (tPos->getPosition() == initialPos) {
                    return "manageObserveTypeWall";
                }
                else {
                    return "generateNavmeshInitialPosTypeWall";
                }
            }
            else {
                return "nextWpt";
            }
        }
    }
    else {
        /* TODO: Gestionar tambien SLEEP */
        if (type == EType::FLOOR) {
            return "nextWpt";
        }
        else if (type == EType::WALL) {
            TCompTransform * tPos = get<TCompTransform>();
            if (tPos->getPosition() == initialPos) {
                return "manageObserveTypeWall";
            }
            else {
                return "generateNavmeshInitialPosTypeWall";
            }
        }
        else {
            return "nextWpt";
        }
    }
}

void TCompAIMimetic::registerMsgs()
{
    DECL_MSG(TCompAIMimetic, TMsgScenePaused, onMsgScenePaused);
    DECL_MSG(TCompAIMimetic, TMsgAIPaused, onMsgAIPaused);
    DECL_MSG(TCompAIMimetic, TMsgEntityCreated, onMsgEntityCreated);
    DECL_MSG(TCompAIMimetic, TMsgPlayerDead, onMsgPlayerDead);
    DECL_MSG(TCompAIMimetic, TMsgEnemyStunned, onMsgMimeticStunned);
    DECL_MSG(TCompAIMimetic, TMsgNoiseMade, onMsgNoiseListened);
    DECL_MSG(TCompAIMimetic, TMsgPhysxContact, onMsgPhysxContact);
	DECL_MSG(TCompAIMimetic, TMsgAnimationCompleted, onMsgAnimationCompleted);
}

void TCompAIMimetic::loadActions() {
    actions_initializer.clear();

}

void TCompAIMimetic::loadConditions() {
    conditions_initializer.clear();

}

void TCompAIMimetic::loadAsserts() {
    asserts_initializer.clear();

}

/* ACTIONS */

BTNode::ERes TCompAIMimetic::actionStunned(float dt)
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

BTNode::ERes TCompAIMimetic::actionExplode(float dt)
{
    CHandle(this).getOwner().destroy();
    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIMimetic::actionResetObserveVariables(float dt)
{
    timerWaitingInObservation = 0.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionWaitOnWall(float dt) {

	TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
	myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE_WALL);
	if (!isPlayerOnLaser()) {
		return BTNode::ERes::STAY;
	}
	else {
		CEntity *e = EngineEntities.getPlayerHandle();
		TCompTransform * c_trans = e->get<TCompTransform>();
		lastPlayerKnownPos = c_trans->getPosition();
		return BTNode::ERes::LEAVE;
	}
	
}

BTNode::ERes TCompAIMimetic::actionObserveLeft(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE_WALL);
	return BTNode::ERes::STAY;
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

BTNode::ERes TCompAIMimetic::actionObserveRight(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
	myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE_WALL);
	return BTNode::ERes::STAY;
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

BTNode::ERes TCompAIMimetic::actionWaitObserving(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE_WALL);
    timerWaitingInObservation += dt;

    if (timerWaitingInObservation < 2.f) {
        return BTNode::ERes::STAY;
    }
    else {
        timerWaitingInObservation = 0.f;
        return BTNode::ERes::LEAVE;
    }
}

BTNode::ERes TCompAIMimetic::actionSetActive(float dt)
{
    isActive = true;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionSetActiveOnWall(float dt) {

	TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();

	if (!myAnimator->isPlayingAnimation((TCompAnimator::EAnimation)TCompMimeticAnimator::EAnimation::WAKE_UP) && !wakeUpAnimationCompleted) {	
		myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::WAKE_UP);

	}
	if (!myAnimator->isPlayingAnimation((TCompAnimator::EAnimation)TCompMimeticAnimator::EAnimation::JUMP_TO_WALL) && !wakeUpJumpAnimationCompleted && wakeUpAnimationCompleted) {
		setLaserState(false);
		myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE);
		myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::JUMP_TO_WALL);

	}
	
	if (wakeUpJumpAnimationCompleted) {

		resetAnimationCompletedBooleans();

		TCompRigidbody *tCollider = get<TCompRigidbody>();
		tCollider->setNormalGravity(VEC3(0, -9.8f, 0));
		isActive = true;

		return BTNode::ERes::LEAVE;
	}
	else {
		return BTNode::ERes::STAY;
	}
	
}

BTNode::ERes TCompAIMimetic::actionJumpFloor(float dt)
{
	//TO-DO: Erase this shiat
    return BTNode::ERes::LEAVE;
    
    
}

BTNode::ERes TCompAIMimetic::actionGenerateNavmeshWpt(float dt)
{
    TCompTransform *tTransform = get<TCompTransform>();
    generateNavmesh(tTransform->getPosition(), _waypoints[currentWaypoint].position);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionGoToWpt(float dt)
{

    if (!isCurrentDestinationReachable() && _waypoints.size() > 1)
        return BTNode::ERes::LEAVE;

    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::WALK);
    return moveToPoint(speed, rotationSpeedPatrolling, getWaypoint().position, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIMimetic::actionResetTimerWaiting(float dt)
{
    timerWaitingInWpt = 0.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionWaitInWpt(float dt)
{
    if (!isCurrentDestinationReachable() && _waypoints.size() > 1)
        return BTNode::ERes::LEAVE;

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

BTNode::ERes TCompAIMimetic::actionNextWpt(float dt)
{
    timerWaitingInWpt = 0.f;
    currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionSleep(float dt)
{
    return BTNode::STAY;
}

BTNode::ERes TCompAIMimetic::actionWakeUp(float dt)
{
    isSlept = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionSuspect(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE);
    TCompEmissionController *eController = get<TCompEmissionController>();
    eController->blend(enemyColor.colorSuspect, 0.1f);
    // chase
    TCompTransform *mypos = get<TCompTransform>();
    CEntity *player = EngineEntities.getPlayerHandle();
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

BTNode::ERes TCompAIMimetic::actionRotateToNoiseSource(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::TURN_LEFT);
    TCompTransform *myPos = get<TCompTransform>();
    bool isInObjective = rotateTowardsVec(noiseSource, rotationSpeedNoise, dt);
    return isInObjective ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIMimetic::actionGenerateNavmeshPlayerLastPos(float dt)
{
    TCompTransform *tpos = get<TCompTransform>();
    generateNavmesh(tpos->getPosition(), lastPlayerKnownPos);

    lastPlayerKnownPos = isCurrentDestinationReachable() ? lastPlayerKnownPos : navmeshPath[navmeshPath.size() - 1];

    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionResetVariablesChase(float dt)
{
    goingInactive = false;
    isActive = true;
    hasHeardArtificialNoise = false;
    hasHeardNaturalNoise = false;

    /* Noise emitter */
    TCompNoiseEmitter * noiseEmitter = get<TCompNoiseEmitter>();
    noiseEmitter->makeNoise(20.f, .4f, true, false, true);

    TCompTransform *tpos = get<TCompTransform>();
    CEntity *player = EngineEntities.getPlayerHandle();
    TCompTransform *ppos = player->get<TCompTransform>();

    generateNavmesh(tpos->getPosition(), ppos->getPosition());

    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionRotateTowardsPlayerWithNoise(float dt)
{
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::TURN_LEFT);
    if (isEntityInFov(entityToChase, fov, maxChaseDistance)) {
        CEntity* player = EngineEntities.getPlayerHandle();
        TCompTransform* ppos = player->get<TCompTransform>();
        rotateTowardsVec(ppos->getPosition(), rotationSpeedChase, dt);
        TCompTransform* mypos = get<TCompTransform>();
        if (lastPlayerKnownPos != ppos->getPosition()) {
            generateNavmesh(mypos->getPosition(), ppos->getPosition());
            lastPlayerKnownPos = ppos->getPosition();
        }
        return BTNode::ERes::STAY;
    }
    else {
        //TODO: Testear a ver si va bene. Idea, no marcamos la posicion del player puesto que hemos salido del nodo sin poder alcanzarle

        /* Cancel noise emitter */
        TCompNoiseEmitter * noiseEmitter = get<TCompNoiseEmitter>();
        noiseEmitter->makeNoise(-1.f, 10.f, false, false, true);

        TCompEmissionController * e_controller = get<TCompEmissionController>();
        e_controller->blend(enemyColor.colorNormal, 0.1f);
        lastPlayerKnownPos = VEC3::Zero;
        return BTNode::ERes::LEAVE;
    }
}

BTNode::ERes TCompAIMimetic::actionChasePlayerWithNoise(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    

    TCompTransform *mypos = get<TCompTransform>();
    CEntity *player = EngineEntities.getPlayerHandle();
    TCompTransform *ppos = player->get<TCompTransform>();

    hasHeardArtificialNoise = false;
    hasHeardNaturalNoise = false;

    TCompEmissionController *eController = get<TCompEmissionController>();
    eController->blend(enemyColor.colorAlert, 0.1f);

    if (lastPlayerKnownPos != VEC3::Zero) {

        /* If we had the player's previous position, know where he is going */
        isLastPlayerKnownDirLeft = mypos->isInLeft(ppos->getPosition() - lastPlayerKnownPos);
    }

    bool pposChanged = lastPlayerKnownPos != ppos->getPosition();
    if (pposChanged) {
        generateNavmesh(mypos->getPosition(), ppos->getPosition());
    }
    
    lastPlayerKnownPos = ppos->getPosition();

    float distToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
    if (!isEntityInFov(entityToChase, fov, maxChaseDistance) || distToPlayer >= maxChaseDistance + 0.5f) {
        myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::RUN);
        TCompEmissionController *eController = get<TCompEmissionController>();
        eController->blend(enemyColor.colorSuspect, 0.1f);

        /* Cancel noise emitter */
        TCompNoiseEmitter * noiseEmitter = get<TCompNoiseEmitter>();
        noiseEmitter->makeNoise(-1.f, 10.f, false, false, true);

        return BTNode::ERes::LEAVE;
    }
    else if (distToPlayer < 1.3f) {

        /* TODO: This fix is temporary */
        if (!pposChanged) {
            myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE);
        }
        else {
            myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::RUN);
        }
        rotateTowardsVec(ppos->getPosition(), rotationSpeedChase, dt);
        return BTNode::ERes::STAY;

    }
    else {
        myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::RUN);
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

BTNode::ERes TCompAIMimetic::actionMarkNoiseAsInactive(float dt)
{
    timerWaitingInNoise = 0.f;
    hasHeardArtificialNoise = false;
    hasHeardNaturalNoise = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionGenerateNavmeshNoiseSource(float dt)
{
    TCompTransform *tpos = get<TCompTransform>();
    noiseSourceChanged = false;
    generateNavmesh(tpos->getPosition(), noiseSource);
    noiseSource = isCurrentDestinationReachable() ? noiseSource : navmeshPath[navmeshPath.size() - 1];
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionGoToNoiseSource(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::RUN);

    CEntity * ePlayer = EngineEntities.getPlayerHandle();
    TCompTransform * ppos = get<TCompTransform>();
    VEC3 pp = ppos->getPosition();

    if (noiseSourceChanged) {
        generateNavmesh(pp, noiseSource);
        noiseSourceChanged = false;
        noiseSource = isCurrentDestinationReachable() ? noiseSource : navmeshPath[navmeshPath.size() - 1];
    }

    if (isEntityInFov(entityToChase, fov - deg2rad(1.f), autoChaseDistance - 1.f)) {
        current = nullptr;
        return BTNode::ERes::LEAVE;
    }

    //VEC3 destination = isCurrentDestinationReachable() ? noiseSource : navmeshPath[navmeshPath.size() - 1];   //TODO: Posible bug si el size de la navmesh es 0? testear

    if (moveToPoint(speed, rotationSpeedPatrolling, noiseSource, dt)) {
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIMimetic::actionWaitInNoiseSource(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE);
    TCompTransform *mypos = get<TCompTransform>();
    VEC3 vp = mypos->getPosition();
    CEntity * ePlayer = EngineEntities.getPlayerHandle();
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

BTNode::ERes TCompAIMimetic::actionGoToPlayerLastPos(float dt)
{

    if (lastPlayerKnownPos == VEC3::Zero) {
        return BTNode::ERes::LEAVE;
    }

    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::RUN);
    if (moveToPoint(speed, rotationSpeedChase, lastPlayerKnownPos, dt)) {
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIMimetic::actionWaitInPlayerLastPos(float dt)
{
    if (lastPlayerKnownPos == VEC3::Zero) {
        return BTNode::ERes::LEAVE;
    }
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

BTNode::ERes TCompAIMimetic::actionSetGoInactive(float dt)
{
    TCompEmissionController *eController = get<TCompEmissionController>();
    eController->blend(enemyColor.colorNormal, 0.1f);
    goingInactive = true;
    suspectO_Meter = 0.f;
    lastPlayerKnownPos = VEC3::Zero;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionGenerateNavmeshInitialPos(float dt)
{
    TCompTransform *tTransform = get<TCompTransform>();
    VEC3 initialPosWithMyY = VEC3(initialPos.x, tTransform->getPosition().y, initialPos.z);
    generateNavmesh(tTransform->getPosition(), initialPosWithMyY);

    initialPos = isCurrentDestinationReachable() ? initialPos : navmeshPath[navmeshPath.size() - 1];
    if (!isCurrentDestinationReachable()) {
        type = EType::FLOOR;    //TODO: Test
    }

    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionGoToInitialPos(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::WALK);
    TCompTransform *mypos = get<TCompTransform>();
    VEC3 initialPosWithMyY = VEC3(initialPos.x, mypos->getPosition().y, initialPos.z);

    return moveToPoint(speed, rotationSpeedPatrolling, initialPosWithMyY, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIMimetic::actionRotateToInitialPos(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::TURN_LEFT);
    TCompTransform *myPos = get<TCompTransform>();
    bool isInObjective = rotateTowardsVec(myPos->getPosition() + initialLookAt, rotationSpeedPatrolling, dt);
    return isInObjective ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

//JUMPING
BTNode::ERes TCompAIMimetic::actionJumpWall(float dt)
{
    //Animation To Change
    TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
    setGravityToFaceWall();
    myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::RETURN_TO_WALL);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionHoldOnWall(float dt)
{
	TCompMimeticAnimator *myAnimator = get<TCompMimeticAnimator>();
	if (myAnimator->isPlayingAnimation((TCompAnimator::EAnimation)TCompMimeticAnimator::EAnimation::RETURN_TO_WALL)) {
		return BTNode::ERes::STAY;
	}
	if (!myAnimator->isPlayingAnimation((TCompAnimator::EAnimation)TCompMimeticAnimator::EAnimation::REST_IN_WALL) && !restAnimationCompleted) {
		myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::IDLE_WALL);
		myAnimator->playAnimation(TCompMimeticAnimator::EAnimation::REST_IN_WALL);
	}
	if (restAnimationCompleted) {
		setLaserState(true);
		resetAnimationCompletedBooleans();

		return BTNode::ERes::LEAVE;
	}
	else {
		return BTNode::ERes::STAY;
	}
    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIMimetic::actionSetInactive(float dt)
{
    isActive = false;
    goingInactive = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIMimetic::actionClosestWpt(float dt)
{
    getClosestWpt();
    return BTNode::ERes::LEAVE;
}


/* CONDITIONS */

bool TCompAIMimetic::conditionHasBeenStunned(float dt)
{
    return hasBeenStunned;
}

bool TCompAIMimetic::conditionIsTypeWall(float dt)
{
    return type == EType::WALL;
}

bool TCompAIMimetic::conditionIsNotPlayerInFovAndNotNoise(float dt)
{
    return !hasHeardNaturalNoise && !hasHeardArtificialNoise && !isEntityInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIMimetic::conditionIsNotPlayerInRaycastAndNotNoise(float dt) {
	return !hasHeardNaturalNoise && !hasHeardArtificialNoise && !isPlayerOnLaser();
}

bool TCompAIMimetic::conditionIsNotActive(float dt)
{
    return !isActive && !goingInactive;
}

bool TCompAIMimetic::conditionHasWpts(float dt)
{
    return _waypoints.size() > 0;
}

bool TCompAIMimetic::conditionIsTypeFloor(float dt)
{
    return type == EType::FLOOR;
}

bool TCompAIMimetic::conditionIsSlept(float dt)
{
    return isSlept;
}

bool TCompAIMimetic::conditionHasNotWaypoints(float dt)
{
    return _waypoints.size() == 0;
}

bool TCompAIMimetic::conditionNotListenedNoise(float dt)
{
    return hasHeardNaturalNoise || hasHeardArtificialNoise;
}

bool TCompAIMimetic::conditionNotSurePlayerInFov(float dt)
{
    return suspectO_Meter < 1.f && (suspectO_Meter > 0.f || isEntityInFov(entityToChase, fov, maxChaseDistance));
}

bool TCompAIMimetic::conditionHasHeardNaturalNoise(float dt)
{
    return hasHeardNaturalNoise;
}

bool TCompAIMimetic::conditionPlayerSeenForSure(float dt)
{
    return isEntityInFov(entityToChase, fov, maxChaseDistance) && suspectO_Meter >= 1.f;
}

bool TCompAIMimetic::conditionHasHeardArtificialNoise(float dt)
{
    return hasHeardArtificialNoise;
}

bool TCompAIMimetic::conditionIsPlayerInFov(float dt)
{
    return isEntityInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIMimetic::conditionNotGoingInactive(float dt)
{
    return !goingInactive;
}

bool TCompAIMimetic::conditionIsDestUnreachable(float dt)
{
    return !isCurrentDestinationReachable();
}

/* ASSERTS */

bool TCompAIMimetic::assertNotPlayerInFovNorNoise(float dt)
{
    return !hasHeardArtificialNoise && !hasHeardNaturalNoise && !isEntityInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIMimetic::assertNotPlayerInFovForSure(float dt)
{
    return !isEntityInFov(entityToChase, fov, autoChaseDistance);;
}

bool TCompAIMimetic::assertNotPlayerInRaycastNorNoise(float dt) {
	return !hasHeardArtificialNoise && !hasHeardNaturalNoise;
}

bool TCompAIMimetic::assertNotPlayerInFov(float dt)
{
    return !isEntityInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIMimetic::assertNotPlayerInFovNorArtificialNoise(float dt)
{
    return !hasHeardArtificialNoise && !isEntityInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIMimetic::assertCantReachDest(float dt)
{
    dbg("Assert CANT REACH %s\n", !isCurrentDestinationReachable() ? "TRUE" : "FALSE");
    return !isCurrentDestinationReachable();
}

bool TCompAIMimetic::assertCanReachDest(float dt)
{
    dbg("Assert CAN REACH %s\n", isCurrentDestinationReachable() ? "TRUE" : "FALSE");
    return isCurrentDestinationReachable();
}

/* AUX FUNCTIONS */

void TCompAIMimetic::setGravityToFaceWall()
{
    TCompRigidbody * tCollider = get<TCompRigidbody>();
    TCompTransform * tTransform = get<TCompTransform>();

    tCollider->setNormalGravity(tTransform->getFront() * -1 * 9.8f);
}

TCompAIMimetic::EType TCompAIMimetic::parseStringMimeticType(const std::string & typeString)
{
    if (typeString.compare("floor") == 0) {
        return EType::FLOOR;
    }
    else if (typeString.compare("wall") == 0) {
        return EType::WALL;
    }
    else {
        fatal("Mimetic type not recognized");
        return EType::NUM_TYPES;
    }
}

bool TCompAIMimetic::isPlayerOnLaser() {
	if (!laser_handle.isValid()) return false;

	CEntity* e = laser_handle;
	TCompLaser* c_laser = e->get<TCompLaser>();

	if (c_laser == nullptr) return false;

	return c_laser->isPlayerOnLaser();


}

void TCompAIMimetic::playAnimationByName(const std::string & animationName)
{
    TCompMimeticAnimator * myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimationConverted(myAnimator->getAnimationByName(animationName));
}

void TCompAIMimetic::registerLaserHandle(CHandle h_laser) {
	laser_handle = h_laser;
}

void TCompAIMimetic::setLaserState(bool state) {
	CEntity *e = laser_handle;
	TCompRender * render_comp = e->get<TCompRender>();
	TCompLaser * laser_comp = e->get<TCompLaser>();
	laser_comp->paused = !state;
	render_comp->visible = state;
}
void TCompAIMimetic::resetAnimationCompletedBooleans() {

	wakeUpJumpAnimationCompleted = false;
	wakeUpAnimationCompleted = false;

	restJumpAnimationCompleted = false;
	restAnimationCompleted = false;

}

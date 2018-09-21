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
#include "components/comp_hierarchy.h"
#include "components/object_controller/comp_shooter.h"

DECL_OBJ_MANAGER("ai_drone", TCompAIDrone);

void TCompAIDrone::debugInMenu() {

    TCompAIEnemy::debugInMenu();
    for (int i = 1; i < _waypoints.size(); i++) {
        if (currentWaypoint == i) {
            renderLine(_waypoints[i - 1].position, _waypoints[i].position, VEC4(1, 0, 0, 1));
        }
        else {
            renderLine(_waypoints[i - 1].position, _waypoints[i].position, VEC4(0, 1, 0, 1));
        }
    }
    ImGui::DragFloat("Lerp Value", &lerpValue, 0.05f, 0.f, 1.f);
}

void TCompAIDrone::preUpdate(float dt)
{
}

void TCompAIDrone::postUpdate(float dt)
{
}

void TCompAIDrone::load(const json& j, TEntityParseContext& ctx) {

    loadActions();
    loadConditions();
    loadAsserts();
    //TCompIAController::loadTree(j);

    createRoot("drone", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
    addChild("drone", "manageStun", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasBeenStunned, nullptr, nullptr);
    addChild("manageStun", "falling", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionFall, nullptr);
    addChild("manageStun", "explode", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionExplode, nullptr);

    addChild("drone", "endAlert", BTNode::EType::ACTION, (BTCondition)&TCompAIDrone::conditionIsPlayerDead, (BTAction)&TCompAIDrone::actionEndAlert, nullptr);

    addChild("drone", "manageDoPatrol", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
    addChild("manageDoPatrol", "manageChase", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionIsPlayerSeenForSure, nullptr, nullptr);
    addChild("manageChase", "markPlayerAsSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionMarkPlayerAsSeen, nullptr);
    addChild("manageChase", "generateNavmeshChase", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshChase, nullptr);
    addChild("manageChase", "chaseAndShoot", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionChaseAndShoot, nullptr);

    addChild("manageDoPatrol", "manageArtificialNoise", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasHeardArtificialNoise, nullptr, nullptr);
    addChild("manageArtificialNoise", "markArtificialNoiseAsInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionMarkNoiseAsInactive, nullptr);
    addChild("manageArtificialNoise", "generateNavmeshArtificialNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshArtificialNoise, nullptr);
    addChild("manageArtificialNoise", "goToArtificialNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToNoiseSource, (BTAssert)&TCompAIDrone::assertNotPlayerInFovForSureNorNextToNoise);
    addChild("manageArtificialNoise", "waitInNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInNoiseSource, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorOrder);
    addChild("manageArtificialNoise", "closestWptArtificialNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionClosestWpt, nullptr);

    addChild("manageDoPatrol", "manageEnemyOrder", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasReceivedEnemyOrder, nullptr, nullptr);
    addChild("manageEnemyOrder", "markOrderAsReceived", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionMarkOrderAsReceived, nullptr);
    addChild("manageEnemyOrder", "generateNavmeshOrder", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshOrder, nullptr);
    addChild("manageEnemyOrder", "goToOrderPosition", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToOrderPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorArtificialNoise);
    addChild("manageEnemyOrder", "waitInOrderPosition", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInOrderPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorArtificialNoiseNorOrder);
    addChild("manageEnemyOrder", "closestWptOrder", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionClosestWpt, nullptr);

    addChild("manageDoPatrol", "managePlayerLost", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionPlayerHasBeenLost, nullptr, nullptr);
    addChild("managePlayerLost", "generateNavmeshPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshPlayerLost, nullptr);
    addChild("managePlayerLost", "goToPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToPlayerLastPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorArtificialNoise);
    addChild("managePlayerLost", "resetTimerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionResetTimer, nullptr);
    addChild("managePlayerLost", "waitInPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInPlayerLastPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoiseNorOrder);
    addChild("managePlayerLost", "closestWptLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionClosestWpt, nullptr);

    addChild("manageDoPatrol", "manageSuspect", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionIsPlayerInFov, nullptr, nullptr);
    addChild("manageSuspect", "generateNavmeshSuspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshSuspect, nullptr);
    addChild("manageSuspect", "suspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSuspect, (BTAssert)&TCompAIDrone::assertNotHeardArtificialNoise);

    addChild("manageDoPatrol", "manageNaturalNoise", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasHeardNaturalNoise, nullptr, nullptr);
    addChild("manageNaturalNoise", "markNaturalNoiseAsInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionMarkNoiseAsInactive, nullptr);
    addChild("manageNaturalNoise", "rotateToNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionRotateToNoiseSource, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorArtificialNoiseNorOrder);

    addChild("manageDoPatrol", "managePatrol", BTNode::EType::SEQUENCE, nullptr, nullptr, nullptr);
    addChild("managePatrol", "generateNavmeshPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshWpt, nullptr);
    addChild("managePatrol", "goToWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToWpt, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoiseNorOrder);
    addChild("managePatrol", "resetTimerPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionResetTimer, nullptr);
    addChild("managePatrol", "waitInWptPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInWpt, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoiseNorOrder);
    addChild("managePatrol", "nextWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionNextWpt, nullptr);

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

    rotationSpeedChase = deg2rad(rotationSpeedChaseDeg);
    fov = deg2rad(fovDeg);
    currentWaypoint = 0;

    enemyColor.colorNormal = j.count("colorNormal") ? loadVEC4(j["colorNormal"]) : VEC4(1, 1, 1, 1);
    enemyColor.colorSuspect = j.count("colorSuspect") ? loadVEC4(j["colorSuspect"]) : VEC4(1, 1, 0, 1);
    enemyColor.colorAlert = j.count("colorAlert") ? loadVEC4(j["colorAlert"]) : VEC4(1, 0, 0, 1);
    enemyColor.colorDead = j.count("colorDead") ? loadVEC4(j["colorDead"]) : VEC4(0, 0, 0, 0);

    btType = BTType::DRONE;
}

void TCompAIDrone::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
    TCompName *tName = get<TCompName>();
    name = tName->getName();

    TCompRigidbody* tRigidbody = get<TCompRigidbody>();
    tRigidbody->setNormalGravity(VEC3::Zero);

    TCompGroup* myGroup = get<TCompGroup>();
    assert(myGroup != nullptr);
    hLantern = myGroup->getHandleByName("Lantern Drone");
    assert(hLantern.isValid());
    CEntity* lantern = hLantern;
    //TCompGroup* lanternGroup = lantern->get<TCompGroup>();
    //assert(lanternGroup != nullptr);
    hLanternLight = myGroup->getHandleByName("Lantern Drone Light");
    assert(hLanternLight.isValid());
    CEntity* lanternLight = hLanternLight;
    TCompConeOfLightController* cone_of_light = lanternLight->get<TCompConeOfLightController>();
    TCompHierarchy* lantern_hierarchy = lantern->get<TCompHierarchy>();
    float yaw, pitch;
    lantern_hierarchy->getYawPitchRoll(&yaw, &pitch);
    startingPitch = pitch;
    cone_of_light->turnOnLight();

    TCompCollider* myCollider = get<TCompCollider>();
    CPhysicsBox * boxCollider = (CPhysicsBox *)myCollider->config;
    geometrySweep.halfExtents = boxCollider->size;

    myHandle = CHandle(this);
}

void TCompAIDrone::onMsgPlayerDead(const TMsgPlayerDead& msg) {

    alarmEnded = false;
}

void TCompAIDrone::onMsgDroneStunned(const TMsgEnemyStunned & msg)
{
    //hasBeenStunned = true;

    //TCompEmissionController *eController = get<TCompEmissionController>();
    //eController->blend(enemyColor.colorDead, 0.1f);
    //TCompTransform *mypos = get<TCompTransform>();
    //float y, p, r;
    //mypos->getYawPitchRoll(&y, &p, &r);
    //p = p + deg2rad(89.f);
    //mypos->setYawPitchRoll(y, p, r);

    //TCompGroup* cGroup = get<TCompGroup>();
    //CEntity* eCone = cGroup->getHandleByName("Cone of Vision");
    //TCompRender * coneRender = eCone->get<TCompRender>();
    //coneRender->visible = false;

    //lastPlayerKnownPos = VEC3::Zero;

    //current = nullptr;
}

void TCompAIDrone::onMsgNoiseListened(const TMsgNoiseMade & msg)
{
    ///* TODO: code in parent */
    bool isManagingArtificialNoise = isNodeSonOf(current, "goToArtificialNoiseSource");
    bool isWaitingInNoiseSource = isNodeName(current, "waitInArtificialNoiseSource");
    bool isManagingNaturalNoise = isNodeSonOf(current, "manageNaturalNoise");
    bool isChasingPlayer = isNodeSonOf(current, "manageChase");
    bool isManagingOrder = isNodeSonOf(current, "goToOrderPosition");

    bool isEntityInFov = isEntityInFovDrone(entityToChase);

    std::chrono::steady_clock::time_point newNoiseTime = std::chrono::steady_clock::now();

    if (!isChasingPlayer && (!isManagingArtificialNoise || isWaitingInNoiseSource)) {

        TCompTransform* mypos = get<TCompTransform>();
        CEntity* ePlayer = getEntityByName(entityToChase);
        TCompTransform* playerPos = ePlayer->get<TCompTransform>();
        if (!(VEC3::Distance(mypos->getPosition(), playerPos->getPosition()) < autoChaseDistance && isEntityInFov)) {
            if (!isManagingOrder && msg.isArtificialNoise) {
                CEntity* ePlayer = getEntityByName(entityToChase);
                TCompTransform* ppos = ePlayer->get<TCompTransform>();
                float jajaja = VEC3::Distance(noiseSource, ppos->getPosition());
                if (VEC3::Distance(noiseSource, ppos->getPosition()) > 4.f || !isEntityInFov) {
                    //dbg("NOISE LISTENED MOTHAFUCKA %f\n", jajaja);
                    hasHeardArtificialNoise = true;
                }
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

void TCompAIDrone::onMsgOrderReceived(const TMsgOrderReceived & msg)
{

    bool isManagingArtificialNoise = isNodeSonOf(current, "goToArtificialNoiseSource");
    bool isChasingPlayer = isNodeSonOf(current, "manageChase");
    bool isManagingOrder = isNodeSonOf(current, "goToOrderPosition");
    bool isManagingSuspect = isNodeSonOf(current, "manageSuspect");

    std::chrono::steady_clock::time_point newOrderTime = std::chrono::steady_clock::now();

    if (!isChasingPlayer && !isManagingArtificialNoise && !isManagingOrder) {
        if (!isEntityInFovDrone(entityToChase)) {
            hasReceivedOrder = true;
        }
    }

    /* Noise management */
    if (!hOrderSource.isValid() || hNoiseSource == msg.hOrderSource || std::chrono::duration_cast<std::chrono::seconds>(newOrderTime - lastTimeOrderReceived).count() > 1.5f) {

        /* Different noise sources (different enemies) => only hear if 1.5 seconds (hardcoded (TODO: Change)) passed || Same noise source => update noise settings */
        lastTimeOrderReceived = newOrderTime;
        orderPosition = msg.position;
        hOrderSource = msg.hOrderSource;
    }
}

/* TODO: --- */
const std::string TCompAIDrone::getStateForCheckpoint()
{
    /* TODO */
    return "nextWpt";
}

void TCompAIDrone::registerMsgs()
{
    DECL_MSG(TCompAIDrone, TMsgScenePaused, onMsgScenePaused);
    DECL_MSG(TCompAIDrone, TMsgAIPaused, onMsgAIPaused);
    DECL_MSG(TCompAIDrone, TMsgEntityCreated, onMsgEntityCreated);
    DECL_MSG(TCompAIDrone, TMsgPlayerDead, onMsgPlayerDead);
    DECL_MSG(TCompAIDrone, TMsgEnemyStunned, onMsgDroneStunned);
    DECL_MSG(TCompAIDrone, TMsgNoiseMade, onMsgNoiseListened);
    DECL_MSG(TCompAIDrone, TMsgOrderReceived, onMsgOrderReceived);
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

bool TCompAIDrone::moveToDestDrone(VEC3 dest, float speed, float dt, bool alsoHeight)
{
    TCompTransform* mypos = get<TCompTransform>();

    #pragma region Rotation with yaw pitch roll (1)

            bool movingUpDown = false;

            bool hasToPitch;
            int rotationSign;
            VEC3 newSpeed = VEC3::Zero;
            float deltayaw = mypos->getDeltaYawToAimTo(dest);
            if (fabsf(deltayaw) < deg2rad(22.5f) || fabsf(deltayaw) > deg2rad(112.5f)) {
                hasToPitch = true;
                if (fabsf(deltayaw) < deg2rad(22.5f)) {
                    rotationSign = -1;
                    newSpeed = mypos->getFront();
                }
                else {
                    rotationSign = 1;
                    newSpeed = -mypos->getFront();
                }
            }
            else {
                hasToPitch = false;
                if (mypos->isInLeft(dest)) {
                    rotationSign = -1;
                    newSpeed = mypos->getLeft();
                }
                else {
                    rotationSign = 1;
                    newSpeed = -mypos->getLeft();
                }
            }

    #pragma endregion

    #pragma region Movement with 4 axis only

            float xSpeed = currentDirection.x;
            float zSpeed = currentDirection.z;
            newSpeed *= speed;
            float xDirSpeed = newSpeed.x;
            float zDirSpeed = newSpeed.z;
            prevDirection = currentDirection;
            currentDirection = VEC3::Lerp(VEC3(xSpeed, 0, zSpeed), VEC3(xDirSpeed, 0, zDirSpeed), lerpValue);

            bool inVerticalOffset = isRespectingVerticalOffset(mypos->getPosition(), dt);
            bool inHorizontalOffset = isRespectingHorizontalOffset(mypos->getPosition(), dt);

            float upVel = 0;
            int upSign = 0;
            if ((inVerticalOffset && fabsf(dest.y - mypos->getPosition().y) > upSpeed * dt) || !inHorizontalOffset ) {
                /* Adjust heigth (up / down) */
                int upSign = 1;
                if (dest.y < mypos->getPosition().y && inHorizontalOffset) {
                    upSign = -1;
                }
                upVel = upSpeed * upSign;
                movingUpDown = true;
            }
            else if ((inVerticalOffset && fabsf(dest.y - mypos->getPosition().y) != 0 )|| !inHorizontalOffset) {
                /* Adjust heigth (with 100% precission) */
                movingUpDown = true;
            }

            if (movingUpDown && (upVel > 0 || (upVel < 0 && isRespectingHorizontalOffset(VEC3(mypos->getPosition().x ,dest.y, mypos->getPosition().z), dt)))) {

                /* Move vertically */
                mypos->setPosition(mypos->getPosition() + VEC3(0, upVel, 0) * dt);
            }
            else if (movingUpDown && upVel == 0) {

                /* Move vertically 100% adjusted */
                mypos->setPosition(VEC3(mypos->getPosition().x, dest.y, mypos->getPosition().z));
            }
            else {

                /* Move horizontally */
                mypos->setPosition(mypos->getPosition() + (currentDirection)* dt);
            }


    #pragma endregion

    #pragma region Rotation Based on Vel

            float maxAmountToRotate = deg2rad(30.f);
            float myActualSpeed = currentDirection.Length();
            float prevSpeed = prevDirection.Length();
            float diffSpeed = currentDirection.Length() - prevSpeed;

            VEC3 localCurrentDirection = VEC3::TransformNormal(currentDirection, mypos->asMatrix().Invert());
            localCurrentDirection = localCurrentDirection - VEC3(0, localCurrentDirection.y, 0);
            localCurrentDirection.Normalize();

            float maxAmountToRotateInAFrame = maxAmountToRotate * dt;
            float amountToPitch;
            float amountToRoll;
            if (hasToPitch && rotationSign == 1) {
                /* Backwards => mas prioridad al cambio de velocidad */
                amountToPitch = maxAmountToRotateInAFrame * ((myActualSpeed / speed) + (diffSpeed / 0.35f * 10)) * localCurrentDirection.z;
            }
            else {
                /* Forward */
                amountToPitch = maxAmountToRotateInAFrame * (myActualSpeed / speed) * localCurrentDirection.z;
            }
            amountToRoll = maxAmountToRotateInAFrame * 2 * ((myActualSpeed / speed) + diffSpeed / 0.35f) * localCurrentDirection.x;
               
            rotateTowardsVec(dest, rotationSpeedNoise, dt);
            float yaw, pitch, roll;
            mypos->getYawPitchRoll(&yaw, &pitch, &roll);
            
            //yaw = lerp(yaw, yaw + deltayaw, dt);
            if (!movingUpDown) {
                if (fabsf(localCurrentDirection.z) < 0.2f) {
                    pitch = lerp(pitch, 0.f, dt);
                }
                else {
                    pitch = Clamp(pitch - amountToPitch, -maxAmountToRotate, maxAmountToRotate * 2);
                }

                if (fabsf(localCurrentDirection.x) < 0.2f) {
                    roll = lerp(roll, 0.f, dt);
                }
                else {
                    roll = Clamp(roll - amountToRoll, -maxAmountToRotate, maxAmountToRotate);
                }
            }
            else {
                pitch = lerp(pitch, 0.f, dt);
                roll = lerp(roll, 0.f, dt);
            }
            mypos->setYawPitchRoll(yaw, pitch, roll);

    #pragma endregion

    if (alsoHeight) {
        return VEC3::Distance(mypos->getPosition(), dest) < speed * dt;
    }
    else {
        return VEC3::Distance2D(mypos->getPosition(), dest) < speed * dt;
    }
}

void TCompAIDrone::waitInPosDrone(VEC3 dest, float dt, float speed, float rotSpeed, VEC3 lookAt)
{
    TCompTransform* mypos = get<TCompTransform>();
    float distance = VEC3::Distance(dest, mypos->getPosition());
    if (distance != 0) {
        /* 1º llegar a la posicion y estabilizar */
        if (distance > speed * dt) {
            moveToPoint(speed, rotSpeed, dest, dt);
            currentDirection = VEC3::Zero;
        }
        else {
            mypos->setPosition(dest);
        }
    }
    else {
        float deltayaw = 0;
        if (lookAt != VEC3::Zero) {
            deltayaw = mypos->getDeltaYawToAimTo(dest + lookAt);
        }
        if (fabsf(deltayaw) > rotSpeed * dt)
        {
            /* 2º yaw a la rotacion del wpt */
            rotateTowardsVec(dest + lookAt, rotSpeed, dt);
        }
        else {
            /* 3º esperar */
        }
    }

    stabilizeRotations(dt);
}

void TCompAIDrone::moveLanternPatrolling(VEC3 objective, float dt, bool resetPitch)
{
    CEntity* eLantern = hLantern;
    TCompHierarchy* lanternHierarchy = eLantern->get<TCompHierarchy>();
    TCompTransform* lanternPos = eLantern->get<TCompTransform>();
    TCompTransform* myPos = get<TCompTransform>();
    float deltaYaw = lanternPos->getDeltaYawToAimTo(objective);

    float yaw, pitch, roll, yaw_addition;
    lanternHierarchy->getYawPitchRoll(&yaw, &pitch, &roll);
    if (lanternPatrollingLeft) {
        yaw_addition = rotationSpeedLantern * dt;
    }
    else {
        yaw_addition = -rotationSpeedLantern * dt;
    }

    if (resetPitch) {
        if (pitch < startingPitch - rotationSpeedLantern * dt) {
            pitch += rotationSpeedLantern * dt;
        }
        else if (pitch > startingPitch + rotationSpeedLantern * dt) {
            pitch -= rotationSpeedLantern * dt;
        }
        else {
            pitch = startingPitch;
        }
    }


    if (fabsf(yaw) <= deg2rad(30.f) && fabsf(yaw + yaw_addition) > deg2rad(30.f)) {
        yaw_addition = 0;
        lanternPatrollingLeft = !lanternPatrollingLeft;
    }
    else if (fabsf(yaw) > deg2rad(30.f) && fabsf(yaw + yaw_addition) > fabsf(yaw)) {
        yaw_addition = -yaw_addition;
        lanternPatrollingLeft = !lanternPatrollingLeft;
    }

    lanternHierarchy->setYawPitchRoll(yaw + yaw_addition, pitch, roll);
}

bool TCompAIDrone::isEntityInFovDrone(const std::string& entityToChase)
{
    CEntity* lanternLight = hLanternLight;
    TCompConeOfLightController* cone_controller = lanternLight->get<TCompConeOfLightController>();

    CHandle hPlayer = getEntityByName(entityToChase);
    CEntity* ePlayer = hPlayer;

    TCompTempPlayerController *pController = ePlayer->get<TCompTempPlayerController>();

    return cone_controller->playerIlluminated && !pController->isInvisible && !pController->isInNoClipMode &&
        !pController->isMerged && !pController->isDead() && !isEntityHidden(hPlayer);
}

void TCompAIDrone::stabilizeRotations(float dt)
{
    TCompTransform* mypos = get<TCompTransform>();
    float yaw, pitch, roll;
    mypos->getYawPitchRoll(&yaw, &pitch, &roll);
    pitch = lerp(pitch, 0.f, dt);
    roll = lerp(roll, 0.f, dt);
    mypos->setYawPitchRoll(yaw, pitch, roll);
}

bool TCompAIDrone::isRespectingVerticalOffset(VEC3 position, float dt)
{
    TCompCollider* myCollider = get<TCompCollider>();

    std::vector<physx::PxSweepHit> hits;
    bool downSweep = EnginePhysics.Sweep(geometrySweep, position, QUAT(0,0,0,1), VEC3(0, -1, 0), flyingDownOffset + addedOffset, hits);
    if (downSweep) {
        bool isHit = true;
        for (int i = 0; i < hits.size(); i++) {
            if (isHit) {
                isHit = hits[i].distance <= flyingDownOffset;
            }
        }
        downSweep = isHit;
    }
    bool upSweep = EnginePhysics.Sweep(geometrySweep, position + VEC3(0, geometrySweep.halfExtents.y * 2, 0), QUAT(0, 0, 0, 1), VEC3(0, 1, 0), flyingUpOffset + addedOffset, hits);
    if (upSweep) {
        bool isHit = true;
        for (int i = 0; i < hits.size(); i++) {
            isHit = hits[i].distance <= flyingUpOffset;
        }
        upSweep = isHit;
    }

    return !downSweep && !upSweep;
}

bool TCompAIDrone::isRespectingHorizontalOffset(VEC3 position, float dt)
{
    TCompTransform* myTransform = get<TCompTransform>();
    TCompCollider* myCollider = get<TCompCollider>();

    std::vector<physx::PxSweepHit> hits;
    VEC3 normalFront = myTransform->getFront();
    normalFront = normalFront - VEC3(0, normalFront.y, 0);
    normalFront.Normalize();

    bool frontSweep = EnginePhysics.Sweep(geometrySweep, position, QUAT(0, 0, 0, 1), normalFront, flyingDownOffset + addedOffset, hits);
    if (frontSweep) {
        bool isHit = true;
        for (int i = 0; i < hits.size(); i++) {
            if (isHit) {
                isHit = hits[i].distance <= flyingDownOffset;
            }
        }
        frontSweep = isHit;
    }

    return !frontSweep;
}

/* ACTIONS */

BTNode::ERes TCompAIDrone::actionFall(float dt)
{
    TCompRigidbody* myRigidbody = get<TCompRigidbody>();
    if (myRigidbody->is_grounded) {
        return BTNode::ERes::LEAVE;
    }
    else {
        /* TODO: No need to move it, make gravity do the trick */
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIDrone::actionExplode(float dt)
{
    CHandle(this).getOwner().destroy();
    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionEndAlert(float dt)
{
    suspectO_Meter = 0.f;
    TCompEmissionController * e_controller = get<TCompEmissionController>();
    e_controller->blend(enemyColor.colorNormal, 0.1f);
    lastPlayerKnownPos = VEC3::Zero;
    alarmEnded = true;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionMarkPlayerAsSeen(float dt)
{
    TCompEmissionController* e_controller = get<TCompEmissionController>();
    e_controller->blend(enemyColor.colorAlert, 0.1f);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshChase(float dt)
{
    CEntity* eLantern = hLantern;
    TCompHierarchy* lantern_hierarchy = eLantern->get<TCompHierarchy>();
    lerpingStartingRotation = lantern_hierarchy->getRotation();
    timeLerpingLanternRot = 0.f;

    /* Shoot */
    CEntity* eLanternLight = hLanternLight;
    TCompShooter* shooter = eLanternLight->get<TCompShooter>();
    shooter->setIsFiring(true, getEntityByName(entityToChase));
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionChaseAndShoot(float dt)
{
    CEntity* ePlayer = getEntityByName(entityToChase);

    if (isEntityInFovDrone(entityToChase)) {
        TCompTransform* playerTransform = ePlayer->get<TCompTransform>();
        TCompTransform* myPosition = get<TCompTransform>();
        VEC3 ppos = playerTransform->getPosition();

        if (lastPlayerKnownPos != ppos) {
            lastPlayerKnownPos = ppos;
        }

        if (VEC3::Distance2D(ppos, myPosition->getPosition()) < 3.f) {
            stabilizeRotations(dt);
            rotateTowardsVec(lastPlayerKnownPos, rotationSpeedNoise, dt);
        }
        else {
            moveToDestDrone(ppos + upOffset, chaseSpeed, dt);
        }

        /* Point lantern to player */
        CEntity* eLantern = hLantern;
        TCompHierarchy* lanternHierarchy = eLantern->get<TCompHierarchy>();
        QUAT lanternRotationObjective = lanternHierarchy->getRelativeLookAt(ppos);
        lanternHierarchy->setRotation(QUAT::Slerp(lerpingStartingRotation, lanternRotationObjective, timeLerpingLanternRot / timeToLerpLanternRot));
        timeLerpingLanternRot = Clamp(timeLerpingLanternRot + dt, 0.f, timeToLerpLanternRot);

        return BTNode::ERes::STAY;
    }
    else {
        /* Stop shooting */
        TCompEmissionController* e_controller = get<TCompEmissionController>();
        e_controller->blend(enemyColor.colorSuspect, 0.1f);

        CEntity* eLanternLight = hLanternLight;
        TCompShooter* shooter = eLanternLight->get<TCompShooter>();
        shooter->setIsFiring(false, CHandle(ePlayer));
        return BTNode::ERes::LEAVE;
    }
}

BTNode::ERes TCompAIDrone::actionMarkNoiseAsInactive(float dt)
{
    hasHeardNaturalNoise = false;
    hasHeardArtificialNoise = false;
    timerWaitingInNoise = 0;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshArtificialNoise(float dt)
{
    CEntity* eLantern = hLantern;
    TCompHierarchy* lantern_hierarchy = eLantern->get<TCompHierarchy>();
    lerpingStartingRotation = lantern_hierarchy->getRotation();
    timeLerpingLanternRot = 0.f;
    timerWaitingInNoise = 0.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToNoiseSource(float dt)
{
    /* Point lantern to player */
    CEntity* eLantern = hLantern;
    TCompHierarchy* lanternHierarchy = eLantern->get<TCompHierarchy>();
    VEC3 lanternObjective = VEC3::Zero;
    if (hNoiseSource.isValid()) {
        CEntity* eNoiseSource = hNoiseSource;
        TCompTransform * noiseSourcePos = eNoiseSource->get<TCompTransform>();
        lanternObjective = noiseSourcePos->getPosition();
    }
    else {
        lanternObjective = noiseSource;
    }
    QUAT lanternRotationObjective = lanternHierarchy->getRelativeLookAt(lanternObjective);
    lanternHierarchy->setRotation(QUAT::Slerp(lerpingStartingRotation, lanternRotationObjective, timeLerpingLanternRot / timeToLerpLanternRot));
    
    timeLerpingLanternRot = Clamp(timeLerpingLanternRot + dt, 0.f, timeToLerpLanternRot);

    return moveToDestDrone(noiseSource + upOffset, maxSpeed, dt, false) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionWaitInNoiseSource(float dt)
{
    moveLanternPatrolling(noiseSource, dt);
    timerWaitingInNoise += dt;
    if (timerWaitingInNoise < maxTimeWaitingInOrderPos) {
        TCompTransform* mypos = get<TCompTransform>();
        waitInPosDrone(mypos->getPosition(), dt, maxSpeed, deg2rad(30.f));
        return BTNode::ERes::STAY;
    }
    else {
        return BTNode::ERes::LEAVE;
    }
}

BTNode::ERes TCompAIDrone::actionClosestWpt(float dt)
{
    getClosestWpt();
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionMarkOrderAsReceived(float dt)
{
    hasReceivedOrder = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshOrder(float dt)
{
    timerWaitingInOrderPos = 0.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToOrderPos(float dt)
{
    moveLanternPatrolling(orderPosition, dt);
    return moveToDestDrone(orderPosition + upOffset, maxSpeed, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionWaitInOrderPos(float dt)
{
    moveLanternPatrolling(orderPosition, dt);
    timerWaitingInOrderPos += dt;
    if(timerWaitingInOrderPos < maxTimeWaitingInOrderPos){
        waitInPosDrone(orderPosition, dt, maxSpeed, deg2rad(60.f));
        return BTNode::ERes::STAY;
    }
    else {
        return BTNode::ERes::LEAVE;
    }
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshPlayerLost(float dt)
{
    timerWaitingInOrderPos += dt;
    TCompTransform* mypos = get<TCompTransform>();
    VEC3 dir = lastPlayerKnownPos - mypos->getPosition();
    dir = dir - VEC3(0, dir.y, 0);
    dir.Normalize();
    lastPlayerKnownPositionOffset = dir * 3.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToPlayerLastPos(float dt)
{
    moveLanternPatrolling(lastPlayerKnownPos, dt, false);
    return moveToDestDrone(lastPlayerKnownPos + lastPlayerKnownPositionOffset + upOffset, maxSpeed, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionWaitInPlayerLastPos(float dt)
{
    timerWaitingInOrderPos += dt;
    if (timerWaitingInOrderPos < maxTimeWaitingInOrderPos) {
        moveLanternPatrolling(lastPlayerKnownPos, dt, false);
        waitInPosDrone(lastPlayerKnownPos + upOffset, dt, maxSpeed, deg2rad(60.f));
        return BTNode::ERes::STAY;
    }
    else {
        lastPlayerKnownPos = VEC3::Zero;
        TCompEmissionController* e_controller = get<TCompEmissionController>();
        e_controller->blend(enemyColor.colorNormal, 0.1f);
        suspectO_Meter = 0.f;
        return BTNode::ERes::LEAVE;
    }
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshSuspect(float dt)
{
    CEntity* eLantern = hLantern;
    TCompHierarchy* lantern_hierarchy = eLantern->get<TCompHierarchy>();
    lerpingStartingRotation = lantern_hierarchy->getRotation();
    timeLerpingLanternRot = 0.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionSuspect(float dt)
{

    TCompEmissionController * e_controller = get<TCompEmissionController>();
    e_controller->blend(enemyColor.colorSuspect, 0.1f);
    TCompTransform *mypos = get<TCompTransform>();
    CEntity *player = getEntityByName(entityToChase);
    TCompTransform *ppos = player->get<TCompTransform>();

    float distanceToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());

    if (distanceToPlayer <= autoChaseDistance && isEntityInFovDrone(entityToChase)) {
        suspectO_Meter = 1.f;
        rotateTowardsVec(ppos->getPosition(), rotationSpeedNoise, dt);
    }
    else if (distanceToPlayer <= maxChaseDistance && isEntityInFovDrone(entityToChase)) {
        suspectO_Meter = Clamp(suspectO_Meter + dt * incrBaseSuspectO_Meter, 0.f, 1.f);							//TODO: increment more depending distance and noise
        rotateTowardsVec(ppos->getPosition(), rotationSpeedNoise, dt);
    }
    else {
        suspectO_Meter = Clamp(suspectO_Meter - dt * dcrSuspectO_Meter, 0.f, 1.f);
    }

    if (suspectO_Meter <= 0.f || suspectO_Meter >= 1.f) {
        if (suspectO_Meter <= 0) {
            e_controller->blend(enemyColor.colorNormal, 0.1f);
        }
        return BTNode::ERes::LEAVE;
    }
    else {
        stabilizeRotations(dt);
        /* Point lantern to player */
        CEntity* eLantern = hLantern;
        TCompHierarchy* lanternHierarchy = eLantern->get<TCompHierarchy>();
        CEntity* ePlayer = getEntityByName(entityToChase);
        TCompTransform* playerpos = ePlayer->get <TCompTransform>();
        QUAT lanternRotationObjective = lanternHierarchy->getRelativeLookAt(playerpos->getPosition());
        lanternHierarchy->setRotation(QUAT::Slerp(lerpingStartingRotation, lanternRotationObjective, timeLerpingLanternRot / timeToLerpLanternRot));
        timeLerpingLanternRot = Clamp(timeLerpingLanternRot + dt, 0.f, timeToLerpLanternRot);
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIDrone::actionRotateToNoiseSource(float dt)
{
    stabilizeRotations(dt);
    return rotateTowardsVec(noiseSource, rotationSpeedNoise, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshWpt(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToWpt(float dt)
{
    if (moveToDestDrone(getWaypoint().position, maxSpeed, dt)) {
        return BTNode::ERes::LEAVE;
    }
    else {
        TCompTransform* myPos = get<TCompTransform>();
        moveLanternPatrolling(myPos->getPosition() + myPos->getFront(), dt);
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIDrone::actionResetTimer(float dt)
{
    timerWaitingInWpt = 0;
    timerWaitingInUnreachablePoint = 0;
    timerWaitingInNoise = 0;
    timerWaitingInObservation = 0;
    timerWaitingInOrderPos = 0;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionWaitInWpt(float dt)
{
    timerWaitingInWpt += dt;
    if (timerWaitingInWpt < getWaypoint().minTime) {
        waitInPosDrone(getWaypoint().position, dt, maxSpeed, deg2rad(60.f), getWaypoint().lookAt);
        return BTNode::ERes::STAY;
    }
    else {

        return BTNode::ERes::LEAVE;
    }
}

BTNode::ERes TCompAIDrone::actionNextWpt(float dt)
{
    timerWaitingInWpt = 0.f;
    currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
    return BTNode::ERes::LEAVE;
}

/* CONDITIONS */

bool TCompAIDrone::conditionHasBeenStunned(float dt)
{
    return hasBeenStunned;
}

bool TCompAIDrone::conditionIsPlayerDead(float dt)
{
    return !alarmEnded;
}

bool TCompAIDrone::conditionIsPlayerSeenForSure(float dt)
{
    return isEntityInFovDrone(entityToChase) && suspectO_Meter >= 1.f;
}

bool TCompAIDrone::conditionHasHeardArtificialNoise(float dt)
{
    return hasHeardArtificialNoise;
}

bool TCompAIDrone::conditionHasReceivedEnemyOrder(float dt)
{
    return hasReceivedOrder;
}

bool TCompAIDrone::conditionPlayerHasBeenLost(float dt)
{
    return lastPlayerKnownPos != VEC3::Zero;
}

bool TCompAIDrone::conditionIsPlayerInFov(float dt)
{
    return isEntityInFovDrone(entityToChase);
}

bool TCompAIDrone::conditionHasHeardNaturalNoise(float dt)
{
    return hasHeardNaturalNoise;
}

/* ASSERTS */

bool TCompAIDrone::assertNotPlayerInFovForSure(float dt)
{
    TCompTransform* mypos = get<TCompTransform>();
    CEntity* ePlayer = getEntityByName(entityToChase);
    TCompTransform* playerPos = ePlayer->get<TCompTransform>();

    return !(VEC3::Distance(mypos->getPosition(), playerPos->getPosition()) < autoChaseDistance && isEntityInFovDrone(entityToChase));
}

bool TCompAIDrone::assertNotPlayerInFovNorArtificialNoise(float dt)
{
    return !hasHeardArtificialNoise && !isEntityInFovDrone(entityToChase);
}

bool TCompAIDrone::assertNotHeardArtificialNoise(float dt)
{
    return !hasHeardArtificialNoise;
}

bool TCompAIDrone::assertNotPlayerInFovNorNoise(float dt)
{
    return !hasHeardArtificialNoise && !hasHeardNaturalNoise && !isEntityInFovDrone(entityToChase);
}

bool TCompAIDrone::assertNotPlayerInFovForSureNorOrder(float dt)
{
    TCompTransform* mypos = get<TCompTransform>();
    CEntity* ePlayer = getEntityByName(entityToChase);
    TCompTransform* playerPos = ePlayer->get<TCompTransform>();

    return VEC3::Distance(mypos->getPosition(), playerPos->getPosition()) > autoChaseDistance && !isEntityInFovDrone(entityToChase) && !hasReceivedOrder;
}

bool TCompAIDrone::assertNotPlayerInFovNorArtificialNoiseNorOrder(float dt)
{
    return !hasHeardArtificialNoise && !isEntityInFovDrone(entityToChase) && !hasReceivedOrder;
}

bool TCompAIDrone::assertNotPlayerInFovNorNoiseNorOrder(float dt)
{
    return !hasHeardArtificialNoise && !hasHeardNaturalNoise && !isEntityInFovDrone(entityToChase) && !hasReceivedOrder;
}

bool TCompAIDrone::assertNotPlayerInFovNorOrder(float dt)
{
    return !isEntityInFovDrone(entityToChase) && !hasReceivedOrder;
}

bool TCompAIDrone::assertNotPlayerInFovForSureNorNextToNoise(float dt)
{
    TCompTransform* mypos = get<TCompTransform>();
    CEntity* ePlayer = getEntityByName(entityToChase);
    TCompTransform* playerPos = ePlayer->get<TCompTransform>();

    VEC3 sourcePosition = VEC3::Zero;
    if (hNoiseSource.isValid()) {
        CEntity* eNoiseSource = hNoiseSource;
        TCompTransform* sourcePos = eNoiseSource->get<TCompTransform>();
        sourcePosition = sourcePos->getPosition();
    }

    bool isPlayerInFov = isEntityInFovDrone(entityToChase);

    //bool result = !((isPlayerInFov && VEC3::Distance(mypos->getPosition(), playerPos->getPosition()) < autoChaseDistance) || (isPlayerInFov && VEC3::Distance(playerPos->getPosition(), sourcePosition) < 4));
    //dbg("ASSERT RESULT %s with distance %f\n", result ? "TRUE" : "FALSE", lele);
    return !((isPlayerInFov && VEC3::Distance(mypos->getPosition(), playerPos->getPosition()) < autoChaseDistance) || (hNoiseSource.isValid() && isPlayerInFov && VEC3::Distance(playerPos->getPosition(), sourcePosition) < 4));
    //isEntityInFov && isAutoChaseDistance || isEntityInFov && nextToWpt => false

   /* bool result = !(VEC3::Distance(mypos->getPosition(), playerPos->getPosition()) < autoChaseDistance && isEntityInFovDrone(entityToChase) && VEC3::Distance(playerPos->getPosition(), sourcePos->getPosition()) < 4);
    dbg("ASSERT RESULT %s with distance %f\n", result ? "TRUE" : "FALSE", lele);

    return !(VEC3::Distance(mypos->getPosition(), playerPos->getPosition()) < autoChaseDistance && isEntityInFovDrone(entityToChase) && VEC3::Distance(playerPos->getPosition(), sourcePos->getPosition()) < 4);*/
}

/* AUX FUNCTIONS */

void TCompAIDrone::playAnimationByName(const std::string & animationName)
{
    //  
}
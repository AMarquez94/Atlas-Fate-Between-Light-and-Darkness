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
    for (int i = 1; i < _waypoints.size(); i++) {
        renderLine(_waypoints[i - 1].position, _waypoints[i].position, VEC4(0, 1, 0, 1));
    }
    ImGui::DragFloat("Lerp Value", &lerpValue, 0.05f, 0.f, 1.f);

    TCompTransform * tpos = get<TCompTransform>();
    renderLine(tpos->getPosition(), tpos->getPosition() + (currentDirection - prevDirection) * 10, VEC4(1, 0, 0, 1));

    //VEC3 localCurrentDirection = VEC3::Transform(tpos->getFront(), tpos->asMatrix().Invert());
    //renderLine(tpos->getPosition(), tpos->getPosition() + localCurrentDirection, VEC4(1, 0, 0, 1));
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

    addChild("drone", "endAlert", BTNode::EType::ACTION, (BTCondition)&TCompAIDrone::conditionIsPlayerDead, nullptr, nullptr);

    addChild("drone", "manageDoPatrol", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
    addChild("manageDoPatrol", "manageChasePlayer", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionIsPlayerSeenForSure, nullptr, nullptr);
    addChild("manageChasePlayer", "markPlayerAsSeen", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionMarkPlayerAsSeen, nullptr);
    addChild("manageChasePlayer", "generateNavmeshChase", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshChase, nullptr);
    addChild("manageChasePlayer", "chaseAndShoot", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionChaseAndShoot, nullptr);

    addChild("manageDoPatrol", "manageArtificialNoise", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasHeardArtificialNoise, nullptr, nullptr);
    addChild("manageArtificialNoise", "markArtificialNoiseAsInactive", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionMarkNoiseAsInactive, nullptr);
    addChild("manageArtificialNoise", "generateNavmeshArtificialNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshArtificialNoise, nullptr);
    addChild("manageArtificialNoise", "goToNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToNoiseSource, (BTAssert)&TCompAIDrone::assertNotPlayerInFovForSure);
    addChild("manageArtificialNoise", "waitInNoiseSource", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInNoiseSource, (BTAssert)&TCompAIDrone::assertNotPlayerInFovForSure);
    addChild("manageArtificialNoise", "closestWptArtificialNoise", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionClosestWpt, nullptr);

    addChild("manageDoPatrol", "manageEnemyOrder", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasReceivedEnemyOrder, nullptr, nullptr);
    addChild("manageEnemyOrder", "markOrderAsReceived", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionMarkOrderAsReceived, nullptr);
    addChild("manageEnemyOrder", "generateNavmeshOrder", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshOrder, nullptr);
    addChild("manageEnemyOrder", "goToOrderPosition", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToOrderPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorArtificialNoise);
    addChild("manageEnemyOrder", "waitInOrderPosition", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInOrderPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorArtificialNoise);
    addChild("manageEnemyOrder", "closestWptOrder", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionClosestWpt, nullptr);

    addChild("manageDoPatrol", "managePlayerLost", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionPlayerHasBeenLost, nullptr, nullptr);
    addChild("managePlayerLost", "generateNavmeshPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshPlayerLost, nullptr);
    addChild("managePlayerLost", "goToPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToPlayerLastPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorArtificialNoise);
    addChild("managePlayerLost", "resetTimerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionResetTimer, nullptr);
    addChild("managePlayerLost", "waitInPlayerLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInPlayerLastPos, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorArtificialNoise);
    addChild("managePlayerLost", "closestWptLastPos", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::getClosestWpt, nullptr);

    addChild("manageDoPatrol", "manageSuspect", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIDrone::conditionHasHeardNaturalNoise, nullptr, nullptr);
    addChild("manageSuspect", "generateNavmeshSuspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshSuspect, nullptr);
    addChild("manageSuspect", "suspect", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionSuspect, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorArtificialNoise);

    addChild("manageDoPatrol", "managePatrol", BTNode::EType::SEQUENCE, nullptr, nullptr, nullptr);
    addChild("managePatrol", "generateNavmeshPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGenerateNavmeshWpt, nullptr);
    addChild("managePatrol", "goToWpt", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionGoToWpt, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
    addChild("managePatrol", "resetTimerPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionResetTimer, nullptr);
    addChild("managePatrol", "waitInWptPatrol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIDrone::actionWaitInWpt, (BTAssert)&TCompAIDrone::assertNotPlayerInFovNorNoise);
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
    //bool isManagingArtificialNoise = isNodeSonOf(current, "goToArtificialNoiseSource");
    //bool isWaitingInNoiseSource = isNodeName(current, "waitInArtificialNoiseSource");
    //bool isManagingNaturalNoise = isNodeSonOf(current, "manageNaturalNoise");
    //bool isChasingPlayer = isNodeSonOf(current, "manageChase");

    //std::chrono::steady_clock::time_point newNoiseTime = std::chrono::steady_clock::now();

    //if (!isChasingPlayer && (!isManagingArtificialNoise || isWaitingInNoiseSource)) {
    //    if (!isEntityInFov(entityToChase, fov - deg2rad(1.f), autoChaseDistance - 1.f)) {
    //        if (msg.isArtificialNoise) {
    //            hasHeardArtificialNoise = true;
    //        }
    //        else if (!isManagingNaturalNoise && !msg.isArtificialNoise) {
    //            hasHeardNaturalNoise = true;
    //        }
    //    }
    //}

    ///* Noise management */
    //if (!hNoiseSource.isValid() || hNoiseSource == msg.hNoiseSource || isManagingNaturalNoise && msg.isArtificialNoise || std::chrono::duration_cast<std::chrono::seconds>(newNoiseTime - lastTimeNoiseWasHeard).count() > 1.5f) {

    //    /* Different noise sources (different enemies) => only hear if 1.5 seconds (hardcoded (TODO: Change)) passed || Same noise source => update noise settings */
    //    lastTimeNoiseWasHeard = newNoiseTime;
    //    noiseSourceChanged = noiseSource != msg.noiseOrigin;
    //    noiseSource = msg.noiseOrigin;
    //    hNoiseSource = msg.hNoiseSource;
    //}
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

BTNode::ERes TCompAIDrone::actionFall(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionExplode(float dt)
{
    CHandle(this).getOwner().destroy();
    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionEndAlert(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionMarkPlayerAsSeen(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshChase(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionChaseAndShoot(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionMarkNoiseAsInactive(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshArtificialNoise(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToNoiseSource(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionWaitInNoiseSource(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionClosestWpt(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionMarkOrderAsReceived(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshOrder(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToOrderPos(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionWaitInOrderPos(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshPlayerLost(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToPlayerLastPos(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionWaitInPlayerLastPos(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshSuspect(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionSuspect(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionRotateToNoiseSource(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGenerateNavmeshWpt(float dt)
{
    currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionGoToWpt(float dt)
{


    TCompTransform* mypos = get<TCompTransform>();
    if (VEC3::Distance(mypos->getPosition(), getWaypoint().position) < maxSpeed * dt) {
        currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
    }
    else {

        #pragma region Movement with all directions
        /* MOVEMENT WITH ALL DIRECTIONS */
        //float xSpeed = currentDirection.x;
        //float zSpeed = currentDirection.z;

        //VEC3 dir = getWaypoint().position - mypos->getPosition();
        //dir.Normalize();
        //VEC3 newSpeed = dir * maxSpeed;

        //float xDirSpeed = newSpeed.x;
        //float zDirSpeed = newSpeed.z;

        //prevDirection = currentDirection;

        //currentDirection = VEC3::Lerp(VEC3(xSpeed, 0, zSpeed), VEC3(xDirSpeed, 0, zDirSpeed), lerpValue );

        //mypos->setPosition(mypos->getPosition() + currentDirection * dt);
        /* END */
        #pragma endregion

        #pragma region Rotation with yaw pitch roll (1)

            bool hasToPitch;
            int rotationSign;
            VEC3 newSpeed = VEC3::Zero;
            float deltayaw = mypos->getDeltaYawToAimTo(getWaypoint().position);
            if (fabsf(deltayaw) < deg2rad(22.5f) || fabsf(deltayaw) > deg2rad(112.5f)) {
                hasToPitch = true;
                if (fabsf(deltayaw) < deg2rad(22.5f)) {
                    rotationSign = -1;
                    newSpeed = mypos->getFront();
                    dbg("FRONT\n");

                }
                else {
                    rotationSign = 1;
                    newSpeed = -mypos->getFront();
                    dbg("BACK\n");
                }
            }
            else {
                hasToPitch = false;
                if (mypos->isInLeft(getWaypoint().position)) {
                    rotationSign = -1;
                    newSpeed = mypos->getLeft();
                    dbg("LEFT\n");
                }
                else {
                    rotationSign = 1;
                    newSpeed = -mypos->getLeft();
                    dbg("RIGHT\n");

                }
            }

        #pragma endregion

        #pragma region Movement with 4 axis only

            float xSpeed = currentDirection.x;
            float zSpeed = currentDirection.z;
            newSpeed *= maxSpeed;
            float xDirSpeed = newSpeed.x;
            float zDirSpeed = newSpeed.z;
            prevDirection = currentDirection;
            prevPos = mypos->getPosition();
            prevSpeed = actualSpeed;
            currentDirection = VEC3::Lerp(VEC3(xSpeed, 0, zSpeed), VEC3(xDirSpeed, 0, zDirSpeed), lerpValue);
            mypos->setPosition(mypos->getPosition() + currentDirection * dt);
            actualSpeed = currentDirection;

            dbg("====================\n");
            dbg("Current dir: %f, -, %f\n", currentDirection.x, currentDirection.z);
            dbg("Prev    dir: %f, -, %f\n", prevDirection.x, prevDirection.z);


        #pragma endregion

        #pragma region Rotation with yaw pitch roll (2)

            //float maxAmountToRotate = deg2rad(30.f);
            //float myActualSpeed = currentDirection.Length();
            //float prevSpeed = prevDirection.Length();
            //float diffSpeed = myActualSpeed - prevSpeed;

            //float diffx = fabsf(currentDirection.x) - fabsf(prevDirection.x);
            //float diffz = fabsf(currentDirection.z) - fabsf(prevDirection.z);

            //dbg("===========\n");
            //dbg("Diff velocidad %f\n", myActualSpeed - prevSpeed);
            //dbg("   x velocidad %f\n", diffx);
            //dbg("   z velocidad %f\n", diffz);
            //if (fabsf(diffSpeed) > fabsf(maxDifferenceSpeed)) {
            //    maxDifferenceSpeed = diffSpeed;
            //}
            //dbg("Max Diff speed %f\n", maxDifferenceSpeed);
            //float rotationDown = maxAmountToRotate * (myActualSpeed / maxSpeed);

            //float yaw, pitch, roll;
            //mypos->getYawPitchRoll(&yaw, &pitch, &roll);
            //if (hasToPitch) {
            //    yaw = lerp(yaw, yaw + deltayaw, lerpValue);
            //    pitch = lerp(pitch, /*pitch + */rotationDown * rotationSign, lerpValue);
            //    roll = lerp(roll, 0.f, lerpValue);
            //}
            //else {
            //    yaw = lerp(yaw, yaw + deltayaw, lerpValue);
            //    pitch = lerp(pitch, 0.f, lerpValue);
            //    roll = lerp(roll, rotationDown * rotationSign, lerpValue);
            //}

            //dbg("Has to %s\n", hasToPitch ? "pitch" : "roll");

            //mypos->setYawPitchRoll(yaw, pitch, roll);
        #pragma endregion

        #pragma region Rotation Based on Vel

            VEC3 myFront = (mypos->getFront() - VEC3(0,mypos->getFront().y,0)).Normalized();
            float dotProduct = myFront.Dot(currentDirection.Normalized());
            dbg("DOT PRODUCT %f\n", dotProduct);


            float maxAmountToRotate = deg2rad(30.f);
            float myActualSpeed = currentDirection.Length();
            float prevSpeed = prevDirection.Length();
            float diffSpeed = myActualSpeed - prevSpeed;

            float diffx = fabsf(currentDirection.x) - fabsf(prevDirection.x);
            float diffz = fabsf(currentDirection.z) - fabsf(prevDirection.z);

            VEC3 localCurrentDirection = VEC3::TransformNormal(currentDirection, mypos->asMatrix().Invert());
            localCurrentDirection = localCurrentDirection - VEC3(0, localCurrentDirection.y, 0);
            //VEC3 localPreviousDirection = VEC3::TransformNormal(prevDirection, mypos->asMatrix().Invert());

            //float localdiffx = fabsf(localCurrentDirection.x) - fabsf(localPreviousDirection.x);
            //float localdiffz = fabsf(localCurrentDirection.z) - fabsf(localPreviousDirection.z);




            dbg("===========\n");
            dbg("Diff velocidad %f\n", myActualSpeed - prevSpeed);
            dbg("   x(w) velocidad %f\n", diffx);
            dbg("   z(w) velocidad %f\n", diffz);
            dbg("   x(l) velocidad %f\n", localCurrentDirection.Normalized().x);
            dbg("   z(l) velocidad %f\n", localCurrentDirection.Normalized().z);
            if (fabsf(diffSpeed) > fabsf(maxDifferenceSpeed)) {
                maxDifferenceSpeed = diffSpeed;
            }
            dbg("Max Diff speed %f\n", maxDifferenceSpeed);
            float rotationDown = maxAmountToRotate * (myActualSpeed / maxSpeed);

            float maxAmountToRotateInAFrame = maxAmountToRotate * dt;
            float amountToPitch;
            float amountToRoll;
            if (hasToPitch && rotationSign == 1) {
                /* Backwards => mas prioridad al cambio de velocidad */
                amountToPitch = maxAmountToRotateInAFrame * ((myActualSpeed / maxSpeed) + (diffSpeed / 0.35f * 10)) * localCurrentDirection.Normalized().z;
            }
            else {
                amountToPitch = maxAmountToRotateInAFrame * (myActualSpeed / maxSpeed) * localCurrentDirection.Normalized().z;
            }
            amountToRoll = maxAmountToRotateInAFrame * 2 * ((myActualSpeed / maxSpeed) + diffSpeed / 0.35f) * localCurrentDirection.Normalized().x;

            float yaw, pitch, roll;
            mypos->getYawPitchRoll(&yaw, &pitch, &roll);
            yaw = lerp(yaw, yaw + deltayaw, lerpValue);
            if (fabsf(localCurrentDirection.Normalized().z) < 0.3f) {
                pitch = lerp(pitch, 0.f, dt);
            }
            else {
                pitch = Clamp(pitch - amountToPitch, -maxAmountToRotate, maxAmountToRotate * 2);
            }

            if (fabsf(localCurrentDirection.Normalized().x) < 0.3f) {
                roll = lerp(roll, 0.f, dt);
            }
            else {
                roll = Clamp(roll - amountToRoll, -maxAmountToRotate, maxAmountToRotate);
            }
            
            
            //if (hasToPitch) {
            //    yaw = lerp(yaw, yaw + deltayaw, lerpValue);
            //    pitch = lerp(pitch, /*pitch + */rotationDown * rotationSign, lerpValue);
            //    roll = lerp(roll, 0.f, lerpValue);
            //}
            //else {
            //    yaw = lerp(yaw, yaw + deltayaw, lerpValue);
            //    pitch = lerp(pitch, 0.f, lerpValue);
            //    roll = lerp(roll, rotationDown * rotationSign, lerpValue);
            //}

            mypos->setYawPitchRoll(yaw, pitch, roll);
           
        #pragma endregion


        #pragma region Rotation with quaternions
        //VEC3 rotationDir = (getWaypoint().position - mypos->getPosition());
        //rotationDir.Normalize();

        //float maxRotationDown = 0.5f;
        //float myActualSpeed = currentDirection.Length();
        //float myPreviousSpeed = prevDirection.Length();
        //float sign = 1;
        //if (myActualSpeed < myPreviousSpeed) {
        //    sign = -1;
        //}
        //float rotationDown = sign * maxRotationDown * (myActualSpeed / maxSpeed);
        //dbg("myRotationDown: %f\n", rotationDown);
        //    
        //QUAT objective_rotation = createLookAt(mypos->getPosition(), mypos->getPosition() + rotationDir - VEC3(0, rotationDown, 0), VEC3(0,1,0));
        ////mypos->setRotation(objective_rotation);
        //QUAT my_rotation = mypos->getRotation();
        //mypos->setRotation(QUAT::Slerp(my_rotation, objective_rotation, dt));
        #pragma endregion

    }

    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIDrone::actionResetTimer(float dt)
{
    timerWaitingInWpt = 0;
    timerWaitingInUnreachablePoint = 0;
    timerWaitingInNoise = 0;
    timerWaitingInObservation = 0;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionWaitInWpt(float dt)
{
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIDrone::actionNextWpt(float dt)
{
    return BTNode::ERes::LEAVE;
}

/* CONDITIONS */

bool TCompAIDrone::conditionHasBeenStunned(float dt)
{
    return false;
}

bool TCompAIDrone::conditionIsPlayerDead(float dt)
{
    return false;
}

bool TCompAIDrone::conditionIsPlayerSeenForSure(float dt)
{
    return false;
}

bool TCompAIDrone::conditionHasHeardArtificialNoise(float dt)
{
    return false;
}

bool TCompAIDrone::conditionHasReceivedEnemyOrder(float dt)
{
    return false;
}

bool TCompAIDrone::conditionPlayerHasBeenLost(float dt)
{
    return false;
}

bool TCompAIDrone::conditionIsPlayerInFov(float dt)
{
    return false;
}

bool TCompAIDrone::conditionHasHeardNaturalNoise(float dt)
{
    return false;
}

/* ASSERTS */

bool TCompAIDrone::assertNotPlayerInFovForSure(float dt)
{
    return true;
}

bool TCompAIDrone::assertNotPlayerInFovNorArtificialNoise(float dt)
{
    return true;
}

bool TCompAIDrone::assertNotHeardArtificialNoise(float dt)
{
    return true;
}

bool TCompAIDrone::assertNotPlayerInFovNorNoise(float dt)
{
    return true;
}

/* AUX FUNCTIONS */

void TCompAIDrone::playAnimationByName(const std::string & animationName)
{
    TCompMimeticAnimator * myAnimator = get<TCompMimeticAnimator>();
    myAnimator->playAnimationConverted(myAnimator->getAnimationByName(animationName));
}
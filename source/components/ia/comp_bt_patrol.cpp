#include "mcv_platform.h"
#include "comp_bt_patrol.h"
#include "btnode.h"
#include "components/comp_name.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/object_controller/comp_cone_of_light.h"
#include "components/physics/comp_rigidbody.h"
#include "components/physics/comp_collider.h"
#include "components/lighting/comp_emission_controller.h"
#include "physics/physics_collider.h"
#include "render/render_utils.h"
#include "components/ia/comp_patrol_animator.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("ai_patrol", TCompAIPatrol);

void TCompAIPatrol::debugInMenu() {

    TCompAIEnemy::debugInMenu();
}

void TCompAIPatrol::preUpdate(float dt)
{
}

void TCompAIPatrol::postUpdate(float dt)
{
}

void TCompAIPatrol::load(const json& j, TEntityParseContext& ctx) {

    loadActions();
    loadConditions();
    loadAsserts();
    TCompIAController::loadTree(j);
    TCompIAController::loadParameters(j);

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
    startLightsOn = j.value("startLightsOn", false);
    currentWaypoint = 0;

    enemyColor.colorNormal = j.count("colorNormal") ? loadVEC4(j["colorNormal"]) : VEC4(1, 1, 1, 1);
    enemyColor.colorSuspect = j.count("colorSuspect") ? loadVEC4(j["colorSuspect"]) : VEC4(1, 1, 0, 1);
    enemyColor.colorAlert = j.count("colorAlert") ? loadVEC4(j["colorAlert"]) : VEC4(1, 0, 0, 1);
    enemyColor.colorDead = j.count("colorDead") ? loadVEC4(j["colorDead"]) : VEC4(0, 0, 0, 0);

    btType = BTType::PATROL;
}

void TCompAIPatrol::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
    TCompName *tName = get<TCompName>();
    name = tName->getName();

    if (_waypoints.size() == 0) {

        TCompTransform * tPos = get<TCompTransform>();
        Waypoint wpt;
        wpt.position = tPos->getPosition();
        wpt.lookAt = tPos->getFront();
        //wpt.lookAt = trueLookAt != VEC3::Zero ? trueLookAt : tPos->getFront();
        wpt.minTime = 1.f;
        addWaypoint(wpt);
    }

    if (startLightsOn) {
        turnOnLight();
    }

    myHandle = CHandle(this);

    //TCompEmissionController *eController = get<TCompEmissionController>();
    //eController->blend(patrolColor.colorNormal, 0.001f);
}

void TCompAIPatrol::onMsgPlayerDead(const TMsgPlayerDead& msg) {

    alarmEnded = false;
    if (!isStunned()) {

        /* We reset the timer if we are not stunned*/
        current = nullptr;
    }
}

void TCompAIPatrol::onMsgPatrolStunned(const TMsgEnemyStunned & msg)
{
    hasBeenStunned = true;

    TCompEmissionController * e_controller = get<TCompEmissionController>();
    e_controller->blend(enemyColor.colorDead, 0.1f);

    TCompTransform *mypos = get<TCompTransform>();
    float y, p, r;
    mypos->getYawPitchRoll(&y, &p, &r);
    p = p - deg2rad(89.f);
    mypos->setYawPitchRoll(y, p, r);
    turnOffLight();

    TCompGroup* cGroup = get<TCompGroup>();
    CEntity* eCone = cGroup->getHandleByName("Cone of Vision");
    TCompRender * coneRender = eCone->get<TCompRender>();
    coneRender->visible = false;

    lastPlayerKnownPos = VEC3::Zero;

    /* Tell the other patrols I am stunned */
    CEngine::get().getIA().patrolSB.stunnedPatrols.emplace_back(CHandle(this).getOwner());

    current = nullptr;
}

void TCompAIPatrol::onMsgPatrolShadowMerged(const TMsgPatrolShadowMerged & msg)
{
    hasBeenShadowMerged = true;

    TCompEmissionController * e_controller = get<TCompEmissionController>();
    e_controller->blend(enemyColor.colorDead, 0.1f);

    /* Stop telling the other patrols that I am stunned */
    bool found = false;
    std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;
    for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
        if (stunnedPatrols[i] == CHandle(this).getOwner()) {
            found = true;
            stunnedPatrols.erase(stunnedPatrols.begin() + i);
        }
    }

    current = nullptr;
}

void TCompAIPatrol::onMsgPatrolFixed(const TMsgPatrolFixed & msg)
{
    if (isStunned()) {

        hasBeenFixed = true;

        TCompTransform *mypos = get<TCompTransform>();
        float y, p, r;
        mypos->getYawPitchRoll(&y, &p, &r);
        p = p + deg2rad(89.f);
        mypos->setYawPitchRoll(y, p, r);
        turnOnLight();

        TCompGroup* cGroup = get<TCompGroup>();
        CEntity* eCone = cGroup->getHandleByName("Cone of Vision");
        TCompRender * coneRender = eCone->get<TCompRender>();
        coneRender->visible = true;

        /* Stop telling the other patrols that I am stunned */
        bool found = false;
        std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;
        for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
            if (stunnedPatrols[i] == CHandle(this).getOwner()) {
                found = true;
                stunnedPatrols.erase(stunnedPatrols.begin() + i);
            }
        }

        TCompEmissionController * e_controller = get<TCompEmissionController>();
        e_controller->blend(enemyColor.colorNormal, 0.1f);

        current = nullptr;
    }
}

void TCompAIPatrol::onMsgNoiseListened(const TMsgNoiseMade & msg)
{
    assert(arguments.find("fov_actionSuspect_suspect") != arguments.end());
    float fov = deg2rad(arguments["fov_actionSuspect_suspect"].getFloat());
    assert(arguments.find("autoChaseDistance_actionSuspect_suspect") != arguments.end());
    float autoChaseDistance = arguments["autoChaseDistance_actionSuspect_suspect"].getFloat();

    bool isManagingArtificialNoise = isNodeSonOf(current, "goToArtificialNoiseSource");
    bool isWaitingInNoiseSource = isNodeName(current, "waitInArtificialNoiseSource");
    bool isManagingNaturalNoise = isNodeSonOf(current, "manageNaturalNoise");
    bool isChasingPlayer = isNodeSonOf(current, "manageChase");

    std::chrono::steady_clock::time_point newNoiseTime = std::chrono::steady_clock::now();

    if (!isChasingPlayer && (!isManagingArtificialNoise || isWaitingInNoiseSource)) {
        if (!isEntityInFov("The Player", fov - deg2rad(1.f), autoChaseDistance - 1.f)) {
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

void TCompAIPatrol::onMsgLanternsDisable(const TMsgLanternsDisable & msg)
{
    if (msg.disable) {
        turnOffLight();
    }
    disabledLanterns = msg.disable;
}

const std::string TCompAIPatrol::getStateForCheckpoint()
{
    if (current) {
        if (current->getName().compare("stunned") == 0) {
            return "stunned";
        }
        else {
            return "nextWpt";
        }
    }
    else {
        return "nextWpt";
    }
}

void TCompAIPatrol::registerMsgs()
{
    DECL_MSG(TCompAIPatrol, TMsgScenePaused, onMsgScenePaused);
    DECL_MSG(TCompAIPatrol, TMsgAIPaused, onMsgAIPaused);
    DECL_MSG(TCompAIPatrol, TMsgEntityCreated, onMsgEntityCreated);
    DECL_MSG(TCompAIPatrol, TMsgPlayerDead, onMsgPlayerDead);
    DECL_MSG(TCompAIPatrol, TMsgEnemyStunned, onMsgPatrolStunned);
    DECL_MSG(TCompAIPatrol, TMsgPatrolShadowMerged, onMsgPatrolShadowMerged);
    DECL_MSG(TCompAIPatrol, TMsgPatrolFixed, onMsgPatrolFixed);
    DECL_MSG(TCompAIPatrol, TMsgNoiseMade, onMsgNoiseListened);
    DECL_MSG(TCompAIPatrol, TMsgLanternsDisable, onMsgLanternsDisable);
}

void TCompAIPatrol::loadActions() {
    actions_initializer.clear();

    actions_initializer["actionShadowMerged"] = (BTAction)&TCompAIPatrol::actionShadowMerged;
    actions_initializer["actionStunned"] = (BTAction)&TCompAIPatrol::actionStunned;
    actions_initializer["actionFixed"] = (BTAction)&TCompAIPatrol::actionFixed;
    actions_initializer["actionBeginAlert"] = (BTAction)&TCompAIPatrol::actionBeginAlert;
    actions_initializer["actionClosestWpt"] = (BTAction)&TCompAIPatrol::actionClosestWpt;
    actions_initializer["actionEndAlert"] = (BTAction)&TCompAIPatrol::actionEndAlert;
    actions_initializer["actionMarkNoiseAsInactive"] = (BTAction)&TCompAIPatrol::actionMarkNoiseAsInactive;
    actions_initializer["actionGenerateNavmeshNoise"] = (BTAction)&TCompAIPatrol::actionGenerateNavmeshNoise;
    actions_initializer["actionGoToNoiseSource"] = (BTAction)&TCompAIPatrol::actionGoToNoiseSource;
    actions_initializer["actionWaitInNoiseSource"] = (BTAction)&TCompAIPatrol::actionWaitInNoiseSource;
    actions_initializer["actionGenerateNavmeshWpt"] = (BTAction)&TCompAIPatrol::actionGenerateNavmeshWpt;
    actions_initializer["actionGoToWpt"] = (BTAction)&TCompAIPatrol::actionGoToWpt;
    actions_initializer["actionWaitInWpt"] = (BTAction)&TCompAIPatrol::actionWaitInWpt;
    actions_initializer["actionNextWpt"] = (BTAction)&TCompAIPatrol::actionNextWpt;
    actions_initializer["actionSuspect"] = (BTAction)&TCompAIPatrol::actionSuspect;
    actions_initializer["actionMarkPlayerAsSeen"] = (BTAction)&TCompAIPatrol::actionMarkPlayerAsSeen;
    actions_initializer["actionShootInhibitor"] = (BTAction)&TCompAIPatrol::actionShootInhibitor;
    actions_initializer["actionGenerateNavmeshChase"] = (BTAction)&TCompAIPatrol::actionGenerateNavmeshChase;
    actions_initializer["actionChasePlayer"] = (BTAction)&TCompAIPatrol::actionChasePlayer;
    actions_initializer["actionAttack"] = (BTAction)&TCompAIPatrol::actionAttack;
    actions_initializer["actionRotateToNoiseSource"] = (BTAction)&TCompAIPatrol::actionRotateToNoiseSource;
    actions_initializer["actionResetPlayerWasSeenVariables"] = (BTAction)&TCompAIPatrol::actionResetPlayerWasSeenVariables;
    actions_initializer["actionGoToPlayerLastPos"] = (BTAction)&TCompAIPatrol::actionGoToPlayerLastPos;
    actions_initializer["actionLookForPlayer"] = (BTAction)&TCompAIPatrol::actionLookForPlayer;
    actions_initializer["actionGenerateNavmeshGoToPatrol"] = (BTAction)&TCompAIPatrol::actionGenerateNavmeshGoToPatrol;
    actions_initializer["actionGoToPatrol"] = (BTAction)&TCompAIPatrol::actionGoToPatrol;
    actions_initializer["actionFixPatrol"] = (BTAction)&TCompAIPatrol::actionFixPatrol;
    actions_initializer["actionMarkPatrolAsLost"] = (BTAction)&TCompAIPatrol::actionMarkPatrolAsLost;

}

void TCompAIPatrol::loadConditions() {
    conditions_initializer.clear();

    conditions_initializer["conditionManageStun"] = (BTCondition)&TCompAIPatrol::conditionManageStun;
    conditions_initializer["conditionEndAlert"] = (BTCondition)&TCompAIPatrol::conditionEndAlert;
    conditions_initializer["conditionShadowMerged"] = (BTCondition)&TCompAIPatrol::conditionShadowMerged;
    conditions_initializer["conditionFixed"] = (BTCondition)&TCompAIPatrol::conditionFixed;
    conditions_initializer["conditionIsArtificialNoise"] = (BTCondition)&TCompAIPatrol::conditionIsArtificialNoise;
    conditions_initializer["conditionIsNaturalNoise"] = (BTCondition)&TCompAIPatrol::conditionIsNaturalNoise;
    conditions_initializer["conditionPlayerSeen"] = (BTCondition)&TCompAIPatrol::conditionPlayerSeen;
    conditions_initializer["conditionPlayerWasSeen"] = (BTCondition)&TCompAIPatrol::conditionPlayerWasSeen;
    conditions_initializer["conditionPatrolSeen"] = (BTCondition)&TCompAIPatrol::conditionPatrolSeen;
    conditions_initializer["conditionFixPatrol"] = (BTCondition)&TCompAIPatrol::conditionFixPatrol;
    conditions_initializer["conditionGoToWpt"] = (BTCondition)&TCompAIPatrol::conditionGoToWpt;
    conditions_initializer["conditionWaitInWpt"] = (BTCondition)&TCompAIPatrol::conditionWaitInWpt;
    conditions_initializer["conditionChase"] = (BTCondition)&TCompAIPatrol::conditionChase;
    conditions_initializer["conditionPlayerAttacked"] = (BTCondition)&TCompAIPatrol::conditionPlayerAttacked;
}

void TCompAIPatrol::loadAsserts() {
    asserts_initializer.clear();

    asserts_initializer["assertPlayerInFov"] = (BTCondition)&TCompAIPatrol::assertPlayerInFov;
    asserts_initializer["assertPlayerNotInFov"] = (BTCondition)&TCompAIPatrol::assertPlayerNotInFov;
    asserts_initializer["assertPlayerAndPatrolNotInFov"] = (BTCondition)&TCompAIPatrol::assertPlayerAndPatrolNotInFov;
    asserts_initializer["assertNotHeardArtificialNoise"] = (BTCondition)&TCompAIPatrol::assertNotHeardArtificialNoise;
    asserts_initializer["assertNotPlayerInFovNorArtificialNoise"] = (BTCondition)&TCompAIPatrol::assertNotPlayerInFovNorArtificialNoise;
    asserts_initializer["assertPlayerNotInFovNorNoise"] = (BTCondition)&TCompAIPatrol::assertPlayerNotInFovNorNoise;
    asserts_initializer["assertPlayerAndPatrolNotInFovNotNoise"] = (BTCondition)&TCompAIPatrol::assertPlayerAndPatrolNotInFovNotNoise;
}

/* ACTIONS */

BTNode::ERes TCompAIPatrol::actionShadowMerged(float dt)
{
    /* Destroy the entity */
    CHandle(this).getOwner().destroy();
    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPatrol::actionStunned(float dt)
{
    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::IDLE);

    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPatrol::actionFixed(float dt)
{
    hasBeenStunned = false;
    hasBeenFixed = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionBeginAlert(float dt)
{
    turnOnLight();
    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::IDLE);

    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionClosestWpt(float dt)
{
    getClosestWpt();
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionEndAlert(float dt)
{

    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::IDLE);

    turnOffLight();
    suspectO_Meter = 0.f;
    TCompEmissionController * e_controller = get<TCompEmissionController>();
    e_controller->blend(enemyColor.colorNormal, 0.1f);
    lastPlayerKnownPos = VEC3::Zero;
    alarmEnded = true;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionMarkNoiseAsInactive(float dt)
{
    hasHeardArtificialNoise = false;
    hasHeardNaturalNoise = false;
    timerWaitingInNoise = 0.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionGenerateNavmeshNoise(float dt)
{
    TCompTransform *tpos = get<TCompTransform>();
    generateNavmesh(tpos->getPosition(), noiseSource);
    noiseSourceChanged = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionGoToNoiseSource(float dt)
{
    assert(arguments.find("speed_actionGoToNoiseSource_goToArtificialNoiseSource") != arguments.end());
    float speed = arguments["speed_actionGoToNoiseSource_goToArtificialNoiseSource"].getFloat();
    assert(arguments.find("rotationSpeed_actionGoToNoiseSource_goToArtificialNoiseSource") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionGoToNoiseSource_goToArtificialNoiseSource"].getFloat());
    assert(arguments.find("entityToChase_actionGoToNoiseSource_goToArtificialNoiseSource") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionGoToNoiseSource_goToArtificialNoiseSource"].getString();
    assert(arguments.find("autoChaseDistance_actionGoToNoiseSource_goToArtificialNoiseSource") != arguments.end());
    float autoChaseDistance = arguments["autoChaseDistance_actionGoToNoiseSource_goToArtificialNoiseSource"].getFloat();
    assert(arguments.find("fov_actionGoToNoiseSource_goToArtificialNoiseSource") != arguments.end());
    float fov = deg2rad(arguments["fov_actionGoToNoiseSource_goToArtificialNoiseSource"].getFloat());

    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::WALK);

    CEntity * ePlayer = getEntityByName(entityToChase);
    TCompTransform * ppos = get<TCompTransform>();
    VEC3 pp = ppos->getPosition();

    if (noiseSourceChanged) {
        generateNavmesh(pp, noiseSource);
        noiseSourceChanged = false;
    }

    //dbg("Go To Noise Source ");
    if (isEntityInFov(entityToChase, fov - deg2rad(1.f), autoChaseDistance - 1.f)) {
        current = nullptr;
        //dbg("LEAVE (player in fov)\n");
        return BTNode::ERes::LEAVE;
    }

    if (moveToPoint(speed, rotationSpeed, noiseSource, dt)) {
        //dbg("LEAVE (is in position) - ");
        //dbg("MyPos: (%f, %f, %f) - NoiseSource: (%f, %f, %f)\n", pp.x, pp.y, pp.z, noiseSource.x, noiseSource.y, noiseSource.z);
        return BTNode::ERes::LEAVE;
    }
    else {
        //dbg("STAY (not arrived)\n");
        return BTNode::ERes::STAY;
    }
    //return moveToPoint(speed, rotationSpeed, noiseSource, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPatrol::actionWaitInNoiseSource(float dt)
{
    assert(arguments.find("entityToChase_actionWaitInNoiseSource_waitInArtificialNoiseSource") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionWaitInNoiseSource_waitInArtificialNoiseSource"].getString();
    assert(arguments.find("autoChaseDistance_actionWaitInNoiseSource_waitInArtificialNoiseSource") != arguments.end());
    float autoChaseDistance = arguments["autoChaseDistance_actionWaitInNoiseSource_waitInArtificialNoiseSource"].getFloat();
    assert(arguments.find("fov_actionWaitInNoiseSource_waitInArtificialNoiseSource") != arguments.end());
    float fov = deg2rad(arguments["fov_actionWaitInNoiseSource_waitInArtificialNoiseSource"].getFloat());

    TCompTransform *mypos = get<TCompTransform>();
    VEC3 vp = mypos->getPosition();
    CEntity * ePlayer = getEntityByName(entityToChase);
    TCompTransform * ppos = get<TCompTransform>();
    VEC3 pp = ppos->getPosition();
    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::IDLE);

    if (isEntityInFov(entityToChase, fov - deg2rad(1.f), autoChaseDistance - 1.f)) {
        current = nullptr;
        return BTNode::ERes::LEAVE;
    }

    timerWaitingInNoise += dt;
    if (timerWaitingInNoise > 3.f) {
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPatrol::actionGenerateNavmeshWpt(float dt)
{
    TCompTransform *tTransform = get<TCompTransform>();
    generateNavmesh(tTransform->getPosition(), _waypoints[currentWaypoint].position);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionGoToWpt(float dt)
{
    assert(arguments.find("speed_actionGoToWpt_goToWpt") != arguments.end());
    float speed = arguments["speed_actionGoToWpt_goToWpt"].getFloat();
    assert(arguments.find("rotationSpeed_actionGoToWpt_goToWpt") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionGoToWpt_goToWpt"].getFloat());

    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::WALK);

    return moveToPoint(speed, rotationSpeed, getWaypoint().position, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPatrol::actionWaitInWpt(float dt)
{
    assert(arguments.find("rotationSpeed_actionWaitInWpt_waitInWpt") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionWaitInWpt_waitInWpt"].getFloat());

    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::IDLE);

    if (timerWaitingInWpt >= getWaypoint().minTime) {
        return BTNode::ERes::LEAVE;
    }
    else {
        timerWaitingInWpt += dt;
        TCompTransform *mypos = get<TCompTransform>();
        if (rotateTowardsVec(mypos->getPosition() + getWaypoint().lookAt, dt, rotationSpeed)) {
            myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::IDLE);
        }
        else {
            myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::WALK);
        }
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPatrol::actionNextWpt(float dt)
{
    timerWaitingInWpt = 0.f;
    currentWaypoint = (currentWaypoint + 1) % _waypoints.size();
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionSuspect(float dt)
{
    assert(arguments.find("rotationSpeed_actionSuspect_suspect") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionSuspect_suspect"].getFloat());
    assert(arguments.find("fov_actionSuspect_suspect") != arguments.end());
    float fov = deg2rad(arguments["fov_actionSuspect_suspect"].getFloat());
    assert(arguments.find("autoChaseDistance_actionSuspect_suspect") != arguments.end());
    float autoChaseDistance = arguments["autoChaseDistance_actionSuspect_suspect"].getFloat();
    assert(arguments.find("maxChaseDistance_actionSuspect_suspect") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_actionSuspect_suspect"].getFloat();
    assert(arguments.find("dcrSuspectO_Meter_actionSuspect_suspect") != arguments.end());
    float dcrSuspectO_Meter = arguments["dcrSuspectO_Meter_actionSuspect_suspect"].getFloat();
    assert(arguments.find("incrBaseSuspectO_Meter_actionSuspect_suspect") != arguments.end());
    float incrBaseSuspectO_Meter = arguments["incrBaseSuspectO_Meter_actionSuspect_suspect"].getFloat();
    assert(arguments.find("entityToChase_actionSuspect_suspect") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionSuspect_suspect"].getString();
    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::IDLE);

    TCompEmissionController * e_controller = get<TCompEmissionController>();
    e_controller->blend(enemyColor.colorSuspect, 0.1f);
    TCompTransform *mypos = get<TCompTransform>();
    CEntity *player = getEntityByName(entityToChase);
    TCompTransform *ppos = player->get<TCompTransform>();

    /* Distance to player */
    float distanceToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());

    if (distanceToPlayer <= autoChaseDistance && isEntityInFov(entityToChase, fov, maxChaseDistance)) {
        suspectO_Meter = 1.f;
        rotateTowardsVec(ppos->getPosition(), dt, rotationSpeed);
    }
    else if (distanceToPlayer <= maxChaseDistance && isEntityInFov(entityToChase, fov, maxChaseDistance)) {
        suspectO_Meter = Clamp(suspectO_Meter + dt * incrBaseSuspectO_Meter, 0.f, 1.f);							//TODO: increment more depending distance and noise
        rotateTowardsVec(ppos->getPosition(), dt, rotationSpeed);
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
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPatrol::actionMarkPlayerAsSeen(float dt)
{
    assert(arguments.find("entityToChase_actionMarkPlayerAsSeen_markPlayerAsSeen") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionMarkPlayerAsSeen_markPlayerAsSeen"].getString();

    CEntity *player = getEntityByName(entityToChase);
    TCompTransform * ppos = player->get<TCompTransform>();
    lastPlayerKnownPos = ppos->getPosition();
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionShootInhibitor(float dt)
{
    assert(arguments.find("entityToChase_actionShootInhibitor_shootInhibitor") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionShootInhibitor_shootInhibitor"].getString();

    CEntity *player = getEntityByName(entityToChase);
    TCompTempPlayerController *pController = player->get<TCompTempPlayerController>();

    TCompEmissionController *eController = get<TCompEmissionController>();
    eController->blend(enemyColor.colorAlert, 0.1f);

    if (!pController->isInhibited) {

        timeAnimating = 0.0f;

        TMsgInhibitorShot msg;
        msg.h_sender = CHandle(this).getOwner();
        player->sendMsg(msg);
    }
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionGenerateNavmeshChase(float dt)
{
    assert(arguments.find("entityToChase_actionChasePlayer_ChasePlayer") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionChasePlayer_ChasePlayer"].getString();

    CEntity *player = getEntityByName(entityToChase);
    TCompTransform *ppos = player->get<TCompTransform>();

    TCompTransform *tpos = get<TCompTransform>();

    generateNavmesh(tpos->getPosition(), ppos->getPosition());
    navmeshPathPoint = 0;
    recalculateNavmesh = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionChasePlayer(float dt)
{
    assert(arguments.find("speed_actionChasePlayer_ChasePlayer") != arguments.end());
    float speed = arguments["speed_actionChasePlayer_ChasePlayer"].getFloat();
    assert(arguments.find("rotationSpeed_actionChasePlayer_ChasePlayer") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionChasePlayer_ChasePlayer"].getFloat());
    assert(arguments.find("fov_actionChasePlayer_ChasePlayer") != arguments.end());
    float fov = deg2rad(arguments["fov_actionChasePlayer_ChasePlayer"].getFloat());
    assert(arguments.find("maxChaseDistance_actionChasePlayer_ChasePlayer") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_actionChasePlayer_ChasePlayer"].getFloat();
    assert(arguments.find("distToAttack_actionChasePlayer_ChasePlayer") != arguments.end());
    float distToAttack = arguments["distToAttack_actionChasePlayer_ChasePlayer"].getFloat();
    assert(arguments.find("entityToChase_actionChasePlayer_ChasePlayer") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionChasePlayer_ChasePlayer"].getString();

    TCompTransform *mypos = get<TCompTransform>();
    CEntity *player = getEntityByName(entityToChase);
    TCompTransform *ppos = player->get<TCompTransform>();

    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::WALK);

    isStunnedPatrolInFov(fov, maxChaseDistance);

    hasHeardArtificialNoise = false;
    hasHeardNaturalNoise = false;

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
		return BTNode::ERes::LEAVE;
	}
	else if (distToPlayer < distToAttack) {
		return BTNode::ERes::LEAVE;
	}
	else {
    return moveToPoint(speed, rotationSpeed, ppos->getPosition(), dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
	}
}

BTNode::ERes TCompAIPatrol::actionAttack(float dt)
{
    assert(arguments.find("entityToChase_actionAttack_attack") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionAttack_attack"].getString();

    CEntity *player = getEntityByName(entityToChase);
    TCompTransform * ppos = player->get<TCompTransform>();

    /* TODO: always hit at the moment - change this */
    TMsgPlayerHit msg;
    msg.h_sender = CHandle(this).getOwner();
    player->sendMsg(msg);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionRotateToNoiseSource(float dt)
{
    assert(arguments.find("rotationSpeed_actionRotateToNoiseSource_rotateToNoiseSource") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionRotateToNoiseSource_rotateToNoiseSource"].getFloat());

    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::WALK);

    TCompTransform *myPos = get<TCompTransform>();
    bool isInObjective = rotateTowardsVec(noiseSource, rotationSpeed, dt);
    return isInObjective ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPatrol::actionResetPlayerWasSeenVariables(float dt)
{
    amountRotated = 0.f;
    TCompTransform *tpos = get <TCompTransform>();
    generateNavmesh(tpos->getPosition(), lastPlayerKnownPos);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionGoToPlayerLastPos(float dt)
{
    assert(arguments.find("speed_actionGoToPlayerLastPos_goToPlayerLastPos") != arguments.end());
    float speed = arguments["speed_actionGoToPlayerLastPos_goToPlayerLastPos"].getFloat();
    assert(arguments.find("rotationSpeed_actionGoToPlayerLastPos_goToPlayerLastPos") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionGoToPlayerLastPos_goToPlayerLastPos"].getFloat());
    assert(arguments.find("fov_actionGoToPlayerLastPos_goToPlayerLastPos") != arguments.end());
    float fov = deg2rad(arguments["fov_actionGoToPlayerLastPos_goToPlayerLastPos"].getFloat());
    assert(arguments.find("maxChaseDistance_actionGoToPlayerLastPos_goToPlayerLastPos") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_actionGoToPlayerLastPos_goToPlayerLastPos"].getFloat();
    assert(arguments.find("entityToChase_actionGoToPlayerLastPos_goToPlayerLastPos") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionGoToPlayerLastPos_goToPlayerLastPos"].getString();

    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::WALK);

    isStunnedPatrolInFov(fov, maxChaseDistance);

    if (moveToPoint(speed, rotationSpeed, lastPlayerKnownPos, dt)) {
        lastPlayerKnownPos = VEC3::Zero;
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPatrol::actionLookForPlayer(float dt)
{
    assert(arguments.find("rotationSpeed_actionLookForPlayer_lookForPlayerSeen") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionLookForPlayer_lookForPlayerSeen"].getFloat());
    assert(arguments.find("fov_actionLookForPlayer_lookForPlayerSeen") != arguments.end());
    float fov = deg2rad(arguments["fov_actionLookForPlayer_lookForPlayerSeen"].getFloat());
    assert(arguments.find("maxRotationSeekingPlayer_actionLookForPlayer_lookForPlayerSeen") != arguments.end());
    float maxRotationSeekingPlayer = deg2rad(arguments["maxRotationSeekingPlayer_actionLookForPlayer_lookForPlayerSeen"].getFloat());
    assert(arguments.find("maxChaseDistance_actionLookForPlayer_lookForPlayerSeen") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_actionLookForPlayer_lookForPlayerSeen"].getFloat();
    assert(arguments.find("entityToChase_actionLookForPlayer_lookForPlayerSeen") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionLookForPlayer_lookForPlayerSeen"].getString();

    TCompTransform *mypos = get<TCompTransform>();
    CEntity *player = (CEntity *)getEntityByName(entityToChase);
    TCompTransform *ppos = player->get<TCompTransform>();
    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::IDLE);

    isStunnedPatrolInFov(fov, maxChaseDistance);

    if (amountRotated >= maxRotationSeekingPlayer * 3) {
        suspectO_Meter = 0.f;

        TCompEmissionController *eController = get<TCompEmissionController>();
        eController->blend(enemyColor.colorNormal, 0.1f);
        amountRotated = 0.f;
        return BTNode::ERes::LEAVE;
    }
    else {

        float y, p, r;
        mypos->getYawPitchRoll(&y, &p, &r);

        amountRotated += rotationSpeed * dt;

        if (amountRotated < maxRotationSeekingPlayer) {
            if (isLastPlayerKnownDirLeft)
            {
                y += rotationSpeed * dt;
            }
            else
            {
                y -= rotationSpeed * dt;
            }
        }
        else {
            if (isLastPlayerKnownDirLeft)
            {
                y -= rotationSpeed * dt;
            }
            else
            {
                y += rotationSpeed * dt;
            }
        }
        mypos->setYawPitchRoll(y, p, r);

        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPatrol::actionGenerateNavmeshGoToPatrol(float dt)
{
    TCompTransform *tTransform = get<TCompTransform>();
    generateNavmesh(tTransform->getPosition(), lastStunnedPatrolKnownPos);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionGoToPatrol(float dt)
{
    assert(arguments.find("speed_actionGoToPatrol_goToPatrol") != arguments.end());
    float speed = arguments["speed_actionGoToPatrol_goToPatrol"].getFloat();
    assert(arguments.find("rotationSpeed_actionGoToPatrol_goToPatrol") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionGoToPatrol_goToPatrol"].getFloat());
    assert(arguments.find("distToAttack_actionGoToPatrol_goToPatrol") != arguments.end());
    float distToAttack = arguments["distToAttack_actionGoToPatrol_goToPatrol"].getFloat();

    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::WALK);

    TCompTransform *tpos = get<TCompTransform>();
    if (VEC3::Distance(tpos->getPosition(), lastStunnedPatrolKnownPos) < distToAttack + 1.f) {
        return BTNode::ERes::LEAVE;
    }
    else {
        return moveToPoint(speed, rotationSpeed, lastStunnedPatrolKnownPos, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPatrol::actionFixPatrol(float dt)
{
    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::IDLE);
    CHandle hPatrol = getPatrolInPos(lastStunnedPatrolKnownPos);
    if (hPatrol.isValid()) {
        CEntity* eStunnedPatrol = hPatrol;
        TMsgPatrolFixed msg;
        msg.h_sender = CHandle(this).getOwner();
        eStunnedPatrol->sendMsg(msg);
        lastStunnedPatrolKnownPos = VEC3::Zero;
    }

    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionMarkPatrolAsLost(float dt)
{
    lastStunnedPatrolKnownPos = VEC3::Zero;
    return BTNode::ERes::LEAVE;
}


/* CONDITIONS */

bool TCompAIPatrol::conditionManageStun(float dt)
{
    return hasBeenStunned;
}

bool TCompAIPatrol::conditionEndAlert(float dt)
{
    return !alarmEnded;
}

bool TCompAIPatrol::conditionShadowMerged(float dt)
{
    return hasBeenShadowMerged;
}

bool TCompAIPatrol::conditionFixed(float dt)
{
    return hasBeenFixed;
}

bool TCompAIPatrol::conditionIsArtificialNoise(float dt)
{
    return hasHeardArtificialNoise;
}

bool TCompAIPatrol::conditionIsNaturalNoise(float dt)
{
    return hasHeardNaturalNoise;
}

bool TCompAIPatrol::conditionPlayerSeen(float dt)
{
    assert(arguments.find("entityToChase_conditionPlayerSeen_managePlayerSeen") != arguments.end());
    std::string entityToChase = arguments["entityToChase_conditionPlayerSeen_managePlayerSeen"].getString();
    assert(arguments.find("fov_conditionPlayerSeen_managePlayerSeen") != arguments.end());
    float fov = deg2rad(arguments["fov_conditionPlayerSeen_managePlayerSeen"].getFloat());
    assert(arguments.find("maxChaseDistance_conditionPlayerSeen_managePlayerSeen") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_conditionPlayerSeen_managePlayerSeen"].getFloat();

    return isEntityInFov(entityToChase, fov, maxChaseDistance);
}

bool TCompAIPatrol::conditionPlayerWasSeen(float dt)
{
    return lastPlayerKnownPos != VEC3::Zero;
}

bool TCompAIPatrol::conditionPatrolSeen(float dt)
{
    assert(arguments.find("fov_conditionPatrolSeen_managePatrolSeen") != arguments.end());
    float fov = deg2rad(arguments["fov_conditionPatrolSeen_managePatrolSeen"].getFloat());
    assert(arguments.find("maxChaseDistance_conditionPatrolSeen_managePatrolSeen") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_conditionPatrolSeen_managePatrolSeen"].getFloat();

    return isStunnedPatrolInFov(fov, maxChaseDistance) || lastStunnedPatrolKnownPos != VEC3::Zero;
}

bool TCompAIPatrol::conditionFixPatrol(float dt)
{
    return isStunnedPatrolInPos(lastStunnedPatrolKnownPos);
}

bool TCompAIPatrol::conditionGoToWpt(float dt)
{
    assert(arguments.find("speed_actionGoToWpt_goToWpt") != arguments.end());
    float speed = arguments["speed_actionGoToWpt_goToWpt"].getFloat();
    TCompTransform* mypos = get<TCompTransform>();
    return VEC3::Distance(mypos->getPosition(), _waypoints[currentWaypoint].position) > speed * dt + 0.1f;
}

bool TCompAIPatrol::conditionWaitInWpt(float dt)
{
    return timerWaitingInWpt < _waypoints[currentWaypoint].minTime;
}

bool TCompAIPatrol::conditionChase(float dt)
{
    return suspectO_Meter >= 1.f;
}

bool TCompAIPatrol::conditionPlayerAttacked(float dt)
{
    assert(arguments.find("entityToChase_conditionPlayerAttacked_managePlayerAttacked") != arguments.end());
    std::string entityToChase = arguments["entityToChase_conditionPlayerAttacked_managePlayerAttacked"].getString();
    assert(arguments.find("distToAttack_conditionPlayerAttacked_managePlayerAttacked") != arguments.end());
    float distToAttack = arguments["distToAttack_conditionPlayerAttacked_managePlayerAttacked"].getFloat();

    TCompTransform *mypos = get<TCompTransform>();
    CEntity *player = (CEntity *)getEntityByName(entityToChase);
    TCompTransform *ppos = player->get<TCompTransform>();

    float distToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
    return distToPlayer < distToAttack;
}

/* ASSERTS */

bool TCompAIPatrol::assertPlayerInFov(float dt)
{
    assert(arguments.find("fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float fov = deg2rad(arguments["fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat());
    assert(arguments.find("maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat();

    return isEntityInFov("The Player", fov, maxChaseDistance);
}

bool TCompAIPatrol::assertPlayerNotInFov(float dt)
{
    assert(arguments.find("fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float fov = deg2rad(arguments["fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat());
    assert(arguments.find("maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat();

    return !isEntityInFov("The Player", fov, maxChaseDistance);
}

bool TCompAIPatrol::assertPlayerAndPatrolNotInFov(float dt)
{
    assert(arguments.find("fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float fov = deg2rad(arguments["fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat());
    assert(arguments.find("maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat();

    return !isEntityInFov("The Player", fov, maxChaseDistance) && !isStunnedPatrolInFov(fov, maxChaseDistance);
}

bool TCompAIPatrol::assertNotHeardArtificialNoise(float dt)
{
    return !hasHeardArtificialNoise;
}

bool TCompAIPatrol::assertNotPlayerInFovNorArtificialNoise(float dt)
{
    assert(arguments.find("fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float fov = deg2rad(arguments["fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat());
    assert(arguments.find("maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat();

    return !isEntityInFov("The Player", fov, maxChaseDistance) && !hasHeardArtificialNoise;
}

bool TCompAIPatrol::assertPlayerNotInFovNorNoise(float dt)
{
    assert(arguments.find("fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float fov = deg2rad(arguments["fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat());
    assert(arguments.find("maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat();

    return !isEntityInFov("The Player", fov, maxChaseDistance) && !hasHeardArtificialNoise && !hasHeardNaturalNoise;
}

bool TCompAIPatrol::assertPlayerAndPatrolNotInFovNotNoise(float dt)
{
    assert(arguments.find("fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float fov = deg2rad(arguments["fov_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat());
    assert(arguments.find("maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_assertNotPlayerInFovNorArtificialNoise_rotateToNoiseSource"].getFloat();

    /* return */ bool result = !isEntityInFov("The Player", fov, maxChaseDistance) && !isStunnedPatrolInFov(fov, maxChaseDistance) && !hasHeardArtificialNoise && !hasHeardNaturalNoise;
    //dbg("Player and patrol not in fov nor noise %s\n", result ? "true" : "false");
    return result;
}

/* AUX FUNCTIONS */

void TCompAIPatrol::turnOnLight()
{
    if (!disabledLanterns) {
        TCompGroup* cGroup = get<TCompGroup>();
        CEntity* eCone = cGroup->getHandleByName("FlashLight");
        TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
        cConeController->turnOnLight();
    }
}

void TCompAIPatrol::turnOffLight() {
    TCompGroup* cGroup = get<TCompGroup>();
    CEntity* eCone = cGroup->getHandleByName("FlashLight");
    TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
    cConeController->turnOffLight();
}

bool TCompAIPatrol::isStunnedPatrolInFov(float fov, float maxChaseDistance)
{
    std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;

    bool found = false;

    if (stunnedPatrols.size() > 0) {
        TCompTransform *mypos = get<TCompTransform>();
        for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
            TCompTransform* stunnedPatrol = ((CEntity*)stunnedPatrols[i])->get<TCompTransform>();
            if (mypos->isInFov(stunnedPatrol->getPosition(), fov, deg2rad(89.f))
                && VEC3::Distance(mypos->getPosition(), stunnedPatrol->getPosition()) < maxChaseDistance
                && !isEntityHidden(stunnedPatrols[i])) {
                found = true;
                lastStunnedPatrolKnownPos = stunnedPatrol->getPosition();
            }
        }
    }

    return found;
}

bool TCompAIPatrol::isStunnedPatrolInPos(VEC3 lastPos)
{
    std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;

    bool found = false;

    if (stunnedPatrols.size() > 0) {
        TCompTransform *mypos = get<TCompTransform>();
        for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
            TCompTransform* stunnedPatrol = ((CEntity*)stunnedPatrols[i])->get<TCompTransform>();
            if (stunnedPatrol->getPosition() == lastPos) {
                found = true;
            }
        }
    }

    return found;
}

CHandle TCompAIPatrol::getPatrolInPos(VEC3 lastPos)
{
    std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;

    bool found = false;
    CHandle h_stunnedPatrol;

    if (stunnedPatrols.size() > 0) {
        TCompTransform *mypos = get <TCompTransform>();
        for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
            TCompTransform* stunnedPatrol = ((CEntity*)stunnedPatrols[i])->get<TCompTransform>();
            if (stunnedPatrol->getPosition() == lastPos) {
                found = true;
                h_stunnedPatrol = stunnedPatrols[i];
            }
        }
    }

	return h_stunnedPatrol;
}

void TCompAIPatrol::playAnimationByName(const std::string & animationName)
{
	TCompPatrolAnimator * myAnimator = get<TCompPatrolAnimator>();
	myAnimator->playAnimationConverted(myAnimator->getAnimationByName(animationName));
}
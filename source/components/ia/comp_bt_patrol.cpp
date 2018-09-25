#include "mcv_platform.h"
#include "comp_bt_patrol.h"
#include "btnode.h"
#include "components/comp_name.h"
#include "components/comp_hierarchy.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/object_controller/comp_cone_of_light.h"
#include "components/physics/comp_rigidbody.h"
#include "components/physics/comp_collider.h"
#include "components/skeleton/comp_skeleton.h"
#include "components/lighting/comp_emission_controller.h"
#include "physics/physics_collider.h"
#include "render/render_utils.h"
#include "components/ia/comp_patrol_animator.h"
#include "render/render_objects.h"
#include "components/lighting/comp_fade_controller.h"
#include "entity/entity_parser.h"
#include "components/comp_tags.h"
#include "components/comp_particles.h"

DECL_OBJ_MANAGER("ai_patrol", TCompAIPatrol);

void TCompAIPatrol::debugInMenu() {

    TCompAIEnemy::debugInMenu();
}

void TCompAIPatrol::preUpdate(float dt)
{
    // Dirty and nasty way of doing this.
    TCompSkeleton * skeleton = get<TCompSkeleton>();
    VEC3 pos = skeleton->getBonePosition("Bip001 Laser");
    TCompTransform * my_trans = get<TCompTransform>();

    CEntity * parent = CHandle(this).getOwner();
    TCompGroup * group = parent->get<TCompGroup>();
    if (group) {
        CEntity * light = group->handles[1];
        TCompHierarchy * t_hier = light->get<TCompHierarchy>();
        MAT44 parent_pos = my_trans->asMatrix().Invert();
        VEC3 new_pos = VEC3::Transform(pos, parent_pos);
        t_hier->setPosition(new_pos);
    }
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
    }/*

    if (startLightsOn) {
        turnOnLight();
    }*/

    myHandle = CHandle(this);

    TCompEmissionController *eController = get<TCompEmissionController>();
    if (eController) {
        eController->blend(enemyColor.colorNormal, 0.1f);
    }
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

    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    if (!myAnimator->isPlayingAnimation((TCompAnimator::EAnimation)(TCompPatrolAnimator::EAnimation::DIE)))
        myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::DIE);
    /*TCompTransform *mypos = get<TCompTransform>();
    float y, p, r;
    mypos->getYawPitchRoll(&y, &p, &r);
    p = p - deg2rad(89.f);
    mypos->setYawPitchRoll(y, p, r);*/
    turnOffLight();

    TCompGroup* cGroup = get<TCompGroup>();
    CEntity* eCone = cGroup->getHandleByName("Cone of Vision");
    TCompRender * coneRender = eCone->get<TCompRender>();
    coneRender->visible = false;

    lastPlayerKnownPos = VEC3::Zero;

    /* Tell the other patrols I am stunned */
    CEngine::get().getIA().patrolSB.stunnedPatrols.emplace_back(CHandle(this).getOwner());

    TCompName* tName = get<TCompName>();
    std::string myName = tName->getName();
    myName.erase(remove(myName.begin(), myName.end(), ' '), myName.end());
    std::string params = myName + "()";
    EngineLogic.execEvent(CModuleLogic::Events::PATROL_STUNNED, params);

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

    TCompFadeController * fade_patrol = get<TCompFadeController>();
    fade_patrol->launch(TMsgFadeBody{ false });

    // TO REFACTOR
    // Sets particles and calls the finishing state.
    CHandle patrol = CHandle(this).getOwner();

    Engine.get().getParticles().launchSystem("data/particles/sm_enter_expand_enemy.particles", patrol);
    Engine.get().getParticles().launchSystem("data/particles/sm_enter_splash.particles", patrol);
    Engine.get().getParticles().launchSystem("data/particles/sm_enter_sparks.particles", patrol);

    CHandle my_rigidbody = get<TCompRigidbody>();
    CHandle my_collider = get<TCompCollider>();
    my_rigidbody.destroy();
    my_collider.destroy();

    current = nullptr;
}

void TCompAIPatrol::onMsgPatrolFixed(const TMsgPatrolFixed & msg)
{
    if (isStunned()) {

        hasBeenFixed = true;

        /*TCompTransform *mypos = get<TCompTransform>();
        float y, p, r;
        mypos->getYawPitchRoll(&y, &p, &r);
        p = p + deg2rad(89.f);
        mypos->setYawPitchRoll(y, p, r);*/
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

void TCompAIPatrol::onMsgCinematicState(const TMsgCinematicState & msg)
{
    _enabledCinematicAI = msg.enableCinematic;
    _currentCinematicState = getStateEnumFromString(msg.state);
    setCurrent(nullptr);
}

void TCompAIPatrol::onMsgAnimationCompleted(const TMsgAnimationCompleted& msg) {

	if (msg.animation_name.compare("inhibidor") == 0) {
		inhibitorAnimationCompleted = true;
	}
	if (msg.animation_name.compare("attack") == 0) {
		attackAnimationCompleted = true;
	}
}

void TCompAIPatrol::onMsgWarned(const TMsgWarnEnemy & msg)
{
    current = nullptr;
    TCompEmissionController *eController = get<TCompEmissionController>();
    eController->blend(enemyColor.colorSuspect, 0.1f);
    lastPlayerKnownPos = msg.playerPosition;
}

void TCompAIPatrol::onMsgResetPatrolLights(const TMsgResetPatrolLights & msg)
{
    if (startLightsOn) {
        turnOnLight();
    }
    else {
        turnOffLight();
    }
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
    DECL_MSG(TCompAIPatrol, TMsgCinematicState, onMsgCinematicState);
	DECL_MSG(TCompAIPatrol, TMsgAnimationCompleted, onMsgAnimationCompleted);
	DECL_MSG(TCompAIPatrol, TMsgWarnEnemy, onMsgWarned);
	DECL_MSG(TCompAIPatrol, TMsgResetPatrolLights, onMsgResetPatrolLights);
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
    actions_initializer["actionWarnClosestDrone"] = (BTAction)&TCompAIPatrol::actionWarnClosestDrone;
    actions_initializer["actionRotateTowardsUnreachablePlayer"] = (BTAction)&TCompAIPatrol::actionRotateTowardsUnreachablePlayer;

    actions_initializer["actionDieAnimation"] = (BTAction)&TCompAIPatrol::actionDieAnimation;
    actions_initializer["actionDeadAnimation"] = (BTAction)&TCompAIPatrol::actionDeadAnimation;
    actions_initializer["actionResetBT"] = (BTAction)&TCompAIPatrol::actionResetBT;
    actions_initializer["actionResetInhibitorCinematicTimers"] = (BTAction)&TCompAIPatrol::actionResetInhibitorCinematicTimers;
    actions_initializer["actionWait"] = (BTAction)&TCompAIPatrol::actionWait;
    actions_initializer["actionAnimationShootInhibitor"] = (BTAction)&TCompAIPatrol::actionAnimationShootInhibitor;
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
    conditions_initializer["conditionIsDestUnreachable"] = (BTCondition)&TCompAIPatrol::conditionIsDestUnreachable;

    conditions_initializer["conditionIsCinematic"] = (BTCondition)&TCompAIPatrol::conditionIsCinematic;
    conditions_initializer["conditionDeadCinematic"] = (BTCondition)&TCompAIPatrol::conditionDeadCinematic;
    conditions_initializer["conditionInhibitorCinematic"] = (BTCondition)&TCompAIPatrol::conditionInhibitorCinematic;
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
    asserts_initializer["assertCantReachDest"] = (BTCondition)&TCompAIPatrol::assertCantReachDest;
    asserts_initializer["assertCanReachDest"] = (BTCondition)&TCompAIPatrol::assertCanReachDest;
}

/* ACTIONS */

BTNode::ERes TCompAIPatrol::actionShadowMerged(float dt)
{
    /* The entity will eventually be destroyed by the fade controller */
    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPatrol::actionStunned(float dt)
{
    //Animation To Change
    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::DEAD);
    //CEntity * patrol = CHandle(this).getOwner();
    //patrol->sendMsg(TMsgFadeBody{ true });

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
    sendSuspectingMsg(false);
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
    noiseSource = isCurrentDestinationReachable() ? noiseSource : navmeshPath[navmeshPath.size() - 1];   //TODO: Posible bug si el size de la navmesh es 0? testear

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
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::RUN);

    CEntity * ePlayer = EngineEntities.getPlayerHandle();
    TCompTransform * ppos = get<TCompTransform>();
    VEC3 pp = ppos->getPosition();

    if (noiseSourceChanged) {
        generateNavmesh(pp, noiseSource);
        noiseSourceChanged = false;
        noiseSource = isCurrentDestinationReachable() ? noiseSource : navmeshPath[navmeshPath.size() - 1];   //TODO: Posible bug si el size de la navmesh es 0? testear
    }

    //dbg("Go To Noise Source ");
    if (isEntityInFov(entityToChase, fov - deg2rad(1.f), autoChaseDistance - 1.f)) {
        current = nullptr;
        //dbg("LEAVE (player in fov)\n");
        return BTNode::ERes::LEAVE;
    }

    bool lol = isCurrentDestinationReachable();

    //VEC3 destination = isCurrentDestinationReachable() ? noiseSource : navmeshPath[navmeshPath.size() - 1];   //TODO: Posible bug si el size de la navmesh es 0? testear

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
    CEntity * ePlayer = EngineEntities.getPlayerHandle();
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

    if (!isCurrentDestinationReachable() && _waypoints.size() > 1)
        return BTNode::ERes::LEAVE;

    assert(arguments.find("speed_actionGoToWpt_goToWpt") != arguments.end());
    float speed = arguments["speed_actionGoToWpt_goToWpt"].getFloat();
    assert(arguments.find("rotationSpeed_actionGoToWpt_goToWpt") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionGoToWpt_goToWpt"].getFloat());
    assert(arguments.find("walkingFast_actionGoToWpt_goToWpt") != arguments.end());
    bool walkingFast = deg2rad(arguments["walkingFast_actionGoToWpt_goToWpt"].getBool());

    //Animation To Change
    if (!walkingFast) {
        TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
        myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::WALK);
    }
    else {
        TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
        myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::WALK_FAST);
    }

    return moveToPoint(speed, rotationSpeed, getWaypoint().position, dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPatrol::actionWaitInWpt(float dt)
{
    if (!isCurrentDestinationReachable() && _waypoints.size() > 1)
        return BTNode::ERes::LEAVE;

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
        if (rotateTowardsVec(mypos->getPosition() + getWaypoint().lookAt, rotationSpeed, dt)) {
            myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::IDLE);
        }
        else {
			if(!myAnimator->isPlayingAnimation((TCompAnimator::EAnimation)TCompPatrolAnimator::EAnimation::TURN_LEFT))
				myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::TURN_LEFT);
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
    CEntity *player = EngineEntities.getPlayerHandle();
    TCompTransform *ppos = player->get<TCompTransform>();

    /* Distance to player */
    float distanceToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());

    if (distanceToPlayer <= autoChaseDistance && isEntityInFov(entityToChase, fov, maxChaseDistance)) {
        suspectO_Meter = 1.f;
        rotateTowardsVec(ppos->getPosition(), rotationSpeed, dt);
    }
    else if (distanceToPlayer <= maxChaseDistance && isEntityInFov(entityToChase, fov, maxChaseDistance)) {
        suspectO_Meter = Clamp(suspectO_Meter + dt * incrBaseSuspectO_Meter, 0.f, 1.f);							//TODO: increment more depending distance and noise
        rotateTowardsVec(ppos->getPosition(), rotationSpeed, dt);
    }
    else {
        sendSuspectingMsg(false);
        suspectO_Meter = Clamp(suspectO_Meter - dt * dcrSuspectO_Meter, 0.f, 1.f);
    }

    if (suspectO_Meter <= 0.f || suspectO_Meter >= 1.f) {
        if (suspectO_Meter <= 0) {
            e_controller->blend(enemyColor.colorNormal, 0.1f);
        }
        sendSuspectingMsg(false);
        return BTNode::ERes::LEAVE;
    }
    else {
        sendSuspectingMsg(true);
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPatrol::actionMarkPlayerAsSeen(float dt)
{
    assert(arguments.find("entityToChase_actionMarkPlayerAsSeen_markPlayerAsSeen") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionMarkPlayerAsSeen_markPlayerAsSeen"].getString();

    CEntity *player = EngineEntities.getPlayerHandle();
    TCompTransform * ppos = player->get<TCompTransform>();
    lastPlayerKnownPos = ppos->getPosition();
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionShootInhibitor(float dt)
{
    //play animation shoot inhibitor
    //
	assert(arguments.find("entityToChase_actionShootInhibitor_shootInhibitor") != arguments.end());
	std::string entityToChase = arguments["entityToChase_actionShootInhibitor_shootInhibitor"].getString(); 
    assert(arguments.find("rotationSpeed_actionChasePlayer_ChasePlayer") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionChasePlayer_ChasePlayer"].getFloat());
    assert(arguments.find("distToAttack_actionChasePlayer_ChasePlayer") != arguments.end());
    float distToAttack = arguments["distToAttack_actionChasePlayer_ChasePlayer"].getFloat();

	CEntity *player = EngineEntities.getPlayerHandle();
	TCompTempPlayerController *pController = player->get<TCompTempPlayerController>();

	TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();

	if (pController->isInhibited && !myAnimator->isPlayingAnimation((TCompAnimator::EAnimation)TCompPatrolAnimator::EAnimation::SHOOT_INHIBITOR)) {
		resetAnimationCompletedBooleans();
		TCompEmissionController *eController = get<TCompEmissionController>();
		eController->blend(enemyColor.colorAlert, 0.1f);
		return BTNode::ERes::LEAVE;
	}
	
	if (!myAnimator->isPlayingAnimation((TCompAnimator::EAnimation)TCompPatrolAnimator::EAnimation::SHOOT_INHIBITOR) && !inhibitorAnimationCompleted) {
        TCompTransform * my_pos = get<TCompTransform>();
        CEntity* player = EngineEntities.getPlayerHandle();
        TCompTransform* ppos = player->get<TCompTransform>();
        if (VEC3::Distance(ppos->getPosition(), my_pos->getPosition()) < distToAttack - 0.1f) {
            return BTNode::ERes::LEAVE;
        }
        else {
		    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::SHOOT_INHIBITOR);
        }
	}
	
	if (inhibitorAnimationCompleted) {

		TCompEmissionController *eController = get<TCompEmissionController>();
		eController->blend(enemyColor.colorAlert, 0.1f);
		resetAnimationCompletedBooleans();
		return BTNode::ERes::LEAVE;
	}
	else {
        TCompTransform* ppos = player->get<TCompTransform>();
        rotateTowardsVec(ppos->getPosition(), rotationSpeed, dt);
		return BTNode::ERes::STAY;
	}
    
}

BTNode::ERes TCompAIPatrol::actionGenerateNavmeshChase(float dt)
{
    assert(arguments.find("entityToChase_actionChasePlayer_ChasePlayer") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionChasePlayer_ChasePlayer"].getString();

    CEntity *player = EngineEntities.getPlayerHandle();
    TCompTransform *ppos = player->get<TCompTransform>();

    TCompTransform *tpos = get<TCompTransform>();

    generateNavmesh(tpos->getPosition(), ppos->getPosition());
    navmeshPathPoint = 0;
    recalculateNavmesh = false;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionWarnClosestDrone(float dt)
{
    assert(arguments.find("entityToChase_actionWarnClosestDrone_warnClosestDrone") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionWarnClosestDrone_warnClosestDrone"].getString();

    /* Prepare msg */
    CEntity* player = EngineEntities.getPlayerHandle();
    TCompTransform* ppos = player->get<TCompTransform>();
    TMsgOrderReceived msg;
    msg.hOrderSource = CHandle(this).getOwner();
    msg.position = ppos->getPosition();

    /* Get closest drone (if any) to warn it */
    VHandles v_drones = CTagsManager::get().getAllEntitiesByTag(getID("drone"));
    float min_dist = INFINITY;
    int min_index = 0;
    bool found = false;
    for (int i = 0; i < v_drones.size(); i++) {
        CEntity* drone = v_drones[i];
        TCompTransform* dpos = drone->get<TCompTransform>();
        float distance = VEC3::Distance(ppos->getPosition(), dpos->getPosition());
        if (distance < min_dist) {
            min_dist = distance;
            min_index = i;
            found = true;
        }
    }

    if (found) {
        v_drones[min_index].sendMsg(msg);
    }

    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionRotateTowardsUnreachablePlayer(float dt)
{
    assert(arguments.find("entityToChase_actionRotateTowardsUnreachablePlayer_rotateTowardsUnreachablePlayer") != arguments.end());
    std::string entityToChase = arguments["entityToChase_actionRotateTowardsUnreachablePlayer_rotateTowardsUnreachablePlayer"].getString();
    assert(arguments.find("fov_actionRotateTowardsUnreachablePlayer_rotateTowardsUnreachablePlayer") != arguments.end());
    float fov = deg2rad(arguments["fov_actionRotateTowardsUnreachablePlayer_rotateTowardsUnreachablePlayer"].getFloat());
    assert(arguments.find("maxChaseDistance_actionRotateTowardsUnreachablePlayer_rotateTowardsUnreachablePlayer") != arguments.end());
    float maxChaseDistance = arguments["maxChaseDistance_actionRotateTowardsUnreachablePlayer_rotateTowardsUnreachablePlayer"].getFloat();
    assert(arguments.find("rotationSpeed_actionRotateTowardsUnreachablePlayer_rotateTowardsUnreachablePlayer") != arguments.end());
    float rotationSpeed = deg2rad(arguments["rotationSpeed_actionRotateTowardsUnreachablePlayer_rotateTowardsUnreachablePlayer"].getFloat());

    if (isEntityInFov(entityToChase, fov, maxChaseDistance)) {
        CEntity* player = EngineEntities.getPlayerHandle();
        TCompTransform* ppos = player->get<TCompTransform>();
        rotateTowardsVec(ppos->getPosition(), rotationSpeed, dt);
        TCompTransform* mypos = get<TCompTransform>();
        if (lastPlayerKnownPos != ppos->getPosition()) {
            generateNavmesh(mypos->getPosition(), ppos->getPosition());
        }
        lastPlayerKnownPos = ppos->getPosition();
        return BTNode::ERes::STAY;
    }
    else {
        //TODO: Testear a ver si va bene. Idea, no marcamos la posicion del player puesto que hemos salido del nodo sin poder alcanzarle
        TCompEmissionController * e_controller = get<TCompEmissionController>();
        e_controller->blend(enemyColor.colorNormal, 0.1f);
        lastPlayerKnownPos = VEC3::Zero;
        suspectO_Meter = 0.f;
        return BTNode::ERes::LEAVE;
    }
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
    CEntity *player = EngineEntities.getPlayerHandle();
    TCompTransform *ppos = player->get<TCompTransform>();

    TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::RUN);

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
        return BTNode::ERes::LEAVE;
    }
    else if (distToPlayer < distToAttack) {
        return BTNode::ERes::LEAVE;
    }
    else {
        warnClosestPatrols();
        return moveToPoint(speed, rotationSpeed, ppos->getPosition(), dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPatrol::actionAttack(float dt)
{
	TCompPatrolAnimator *myAnimator = get<TCompPatrolAnimator>();
	if (!myAnimator->isPlayingAnimation((TCompAnimator::EAnimation)TCompPatrolAnimator::EAnimation::ATTACK) && !attackAnimationCompleted) {
		myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::ATTACK);
	}

	if (attackAnimationCompleted) {
		resetAnimationCompletedBooleans();
		return BTNode::ERes::LEAVE;
	}
	else {
		return BTNode::ERes::STAY;
	}

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

    lastPlayerKnownPos = isCurrentDestinationReachable() ? lastPlayerKnownPos : navmeshPath[navmeshPath.size() - 1];

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
    myAnimator->playAnimation(TCompPatrolAnimator::EAnimation::RUN);

    isStunnedPatrolInFov(fov, maxChaseDistance);

    if (moveToPoint(speed, rotationSpeed, lastPlayerKnownPos, dt)) {

        TCompEmissionController *eController = get<TCompEmissionController>();
        eController->blend(enemyColor.colorSuspect, 0.1f);
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
    CEntity *player = (CEntity *)EngineEntities.getPlayerHandle();
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
    if (!isCurrentDestinationReachable()) {
        CHandle patrolToIgnore = getPatrolInPos(lastStunnedPatrolKnownPos);
        ignoredPatrols.push_back(patrolToIgnore);
        lastStunnedPatrolKnownPos = VEC3::Zero;
    }
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

BTNode::ERes TCompAIPatrol::actionDieAnimation(float dt)
{
    TCompPatrolAnimator* my_anim = get<TCompPatrolAnimator>();
    my_anim->playAnimation(TCompPatrolAnimator::DIE);
    TCompEmissionController* my_emission = get<TCompEmissionController>();
    my_emission->blend(enemyColor.colorDead, 0.001f);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionDeadAnimation(float dt)
{
    TCompPatrolAnimator* my_anim = get<TCompPatrolAnimator>();
    my_anim->playAnimation(TCompPatrolAnimator::DEAD);
    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPatrol::actionResetBT(float dt)
{
    setCurrent(nullptr);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionResetInhibitorCinematicTimers(float dt)
{
    _cinematicTimer = 0.f;
    _cinematicMaxTime = 2.f;
    _currentCinematicState = EState::CINEMATIC_DEAD;
    TCompPatrolAnimator* my_anim = get<TCompPatrolAnimator>();
    my_anim->playAnimation(TCompPatrolAnimator::IDLE);
    TCompEmissionController* my_emission = get<TCompEmissionController>();
    my_emission->blend(enemyColor.colorSuspect, 1.5f);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPatrol::actionWait(float dt)
{
    _cinematicTimer += dt;
    if (_cinematicTimer > _cinematicMaxTime) {
        _cinematicTimer = 0.f;
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPatrol::actionAnimationShootInhibitor(float dt)
{
    if (_cinematicTimer == 0) {
        TCompAudio* my_audio = get<TCompAudio>();
        my_audio->playEvent("event:/Sounds/Enemies/Patrol/PatrolInhibitorTest");
    }

    if (_cinematicTimer > 0.6f) {
        _cinematicTimer = 0.f;
        TCompEmissionController* my_emission = get<TCompEmissionController>();
        EngineLogic.execScript("animation_LaunchInhibitor(" + CHandle(this).getOwner().asString() + ")");
        return BTNode::ERes::LEAVE;
    }
    else {
        _cinematicTimer += dt;
        return BTNode::ERes::STAY;
    }



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
    CEntity *player = EngineEntities.getPlayerHandle();
    TCompTransform *ppos = player->get<TCompTransform>();

    float distToPlayer = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
    return distToPlayer < distToAttack;
}

bool TCompAIPatrol::conditionIsDestUnreachable(float dt)
{
    return !isCurrentDestinationReachable();
}

bool TCompAIPatrol::conditionIsCinematic(float dt)
{
    return _enabledCinematicAI;
}

bool TCompAIPatrol::conditionDeadCinematic(float dt)
{
    return _currentCinematicState == TCompAIPatrol::EState::CINEMATIC_DEAD;
}

bool TCompAIPatrol::conditionInhibitorCinematic(float dt)
{
    return _currentCinematicState == TCompAIPatrol::EState::CINEMATIC_INHIBITOR;
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

bool TCompAIPatrol::assertCantReachDest(float dt)
{
    return !isCurrentDestinationReachable();
}

bool TCompAIPatrol::assertCanReachDest(float dt)
{
    return isCurrentDestinationReachable();
}

/* AUX FUNCTIONS */

void TCompAIPatrol::turnOnLight()
{
    if (!disabledLanterns) {
        if (EngineIA.patrolSB.patrolsWithLight.size() < 4) {
            TCompGroup* cGroup = get<TCompGroup>();
            CEntity* eCone = cGroup->getHandleByName("FlashLight");
            TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
            cConeController->turnOnLight();

            TCompParticles* flashParticles = eCone->get<TCompParticles>();
            flashParticles->setSystemState(true);

            bool found = false;
            for (int i = 0; i < EngineIA.patrolSB.patrolsWithLight.size(); i++) {
                if (EngineIA.patrolSB.patrolsWithLight[i] == myHandle.getOwner()) {
                    found = true;
                }
            }

            if (!found) {
                EngineIA.patrolSB.patrolsWithLight.push_back(myHandle.getOwner());
            }
        }
    }
}

void TCompAIPatrol::turnOffLight() {
    TCompGroup* cGroup = get<TCompGroup>();
    CEntity* eCone = cGroup->getHandleByName("FlashLight");
    TCompConeOfLightController* cConeController = eCone->get<TCompConeOfLightController>();
    cConeController->turnOffLight();
    TCompParticles* flashParticles = eCone->get<TCompParticles>();
    flashParticles->setSystemState(false);

    bool found = false;
    for (int i = 0; i < EngineIA.patrolSB.patrolsWithLight.size(); i++) {
        if (EngineIA.patrolSB.patrolsWithLight[i] == myHandle.getOwner()) {
            found = true;
            EngineIA.patrolSB.patrolsWithLight.erase(EngineIA.patrolSB.patrolsWithLight.begin() + i);
        }
    }
}

bool TCompAIPatrol::isStunnedPatrolInFov(float fov, float maxChaseDistance)
{
    std::vector <CHandle> &stunnedPatrols = CEngine::get().getIA().patrolSB.stunnedPatrols;

    bool found = false;

    if (stunnedPatrols.size() > 0) {
        TCompTransform *mypos = get<TCompTransform>();
        TCompCollider * myCollider = get<TCompCollider>();
        CPhysicsCapsule * capsuleCollider = (CPhysicsCapsule *)myCollider->config;
        float myY = mypos->getPosition().y;
        for (int i = 0; i < stunnedPatrols.size() && !found; i++) {
            CEntity* ePatrol = stunnedPatrols[i];
            TCompTransform* stunnedPatrol = ePatrol->get<TCompTransform>();
            float enemyY = stunnedPatrol->getPosition().y;
            if (mypos->isInFov(stunnedPatrol->getPosition(), fov, deg2rad(45.f))
                && VEC3::Distance(mypos->getPosition(), stunnedPatrol->getPosition()) < maxChaseDistance
                && !isEntityHidden(stunnedPatrols[i])
                && fabsf(myY - enemyY) <= 2 * capsuleCollider->height
                && std::find(ignoredPatrols.begin(), ignoredPatrols.end(), stunnedPatrols[i]) == ignoredPatrols.end()) {
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

float TCompAIPatrol::getMaxChaseDistance()
{
    float maxChaseDistance = arguments["maxChaseDistance_actionSuspect_suspect"].getFloat();
    assert(arguments.find("dcrSuspectO_Meter_actionSuspect_suspect") != arguments.end());
    return maxChaseDistance;
}

TCompAIPatrol::EState TCompAIPatrol::getStateEnumFromString(const std::string& stateName)
{
    if (stateName.compare("inhibitor_cinematic") == 0) {
        return TCompAIPatrol::EState::CINEMATIC_INHIBITOR;
    }
    else if (stateName.compare("dead_cinematic") == 0) {
        return TCompAIPatrol::EState::CINEMATIC_DEAD;
    }
    return TCompAIPatrol::EState::NUM_STATES;
}

void TCompAIPatrol::warnClosestPatrols()
{
    std::vector<CHandle> enemies_in_range;

    physx::PxSphereGeometry geometryAttack;
    geometryAttack.radius = 2.f;

    if (geometryAttack.isValid()) {

        TCompTransform* tPos = get<TCompTransform>();

        physx::PxFilterData pxFilterData;
        pxFilterData.word0 = FilterGroup::Enemy;
        physx::PxQueryFilterData PxEnemyWarnFilterData;
        PxEnemyWarnFilterData.data = pxFilterData;
        PxEnemyWarnFilterData.flags = physx::PxQueryFlag::eDYNAMIC;
        std::vector<physx::PxOverlapHit> hits;
        if (EnginePhysics.Overlap(geometryAttack, tPos->getPosition(), hits, PxEnemyWarnFilterData)) {
            for (int i = 0; i < hits.size(); i++) {
                CHandle hitCollider;
                hitCollider.fromVoidPtr(hits[i].actor->userData);
                if (hitCollider.isValid()) {
                    CHandle enemy = hitCollider.getOwner();
                    if (enemy.isValid() && enemy != myHandle.getOwner()) {
                        enemies_in_range.push_back(enemy);
                    }
                }
            }
        }
    }

    for (int i = 0; i < enemies_in_range.size(); i++) {
        CEntity * enemy = enemies_in_range[i];
        TCompTransform *ePos = enemy->get<TCompTransform>();
        TCompTags * eTag = enemy->get<TCompTags>();

        if (eTag->hasTag(getID("patrol"))) {
            TCompAIPatrol* tPatrol = enemy->get<TCompAIPatrol>();
            if (tPatrol->isNodeSonOf(tPatrol->current, "managePatrolGoToWpt")
                || tPatrol->isNodeSonOf(tPatrol->current, "waitInWpt")){
                TMsgWarnEnemy msg;
                msg.playerPosition = lastPlayerKnownPos;
                enemy->sendMsg(msg);
            }
        }
    }
}

void TCompAIPatrol::launchInhibitor()
{
    TEntityParseContext ctxInhibitor;
    ctxInhibitor.entity_starting_the_parse = CHandle(this).getOwner();
    parseScene("data/prefabs/inhibitor.prefab", ctxInhibitor);
    TCompGroup* myGroup = get<TCompGroup>();
    myGroup->add(ctxInhibitor.entities_loaded[0]);
}

void TCompAIPatrol::attackPlayer()
{
    CEntity* player = EngineEntities.getPlayerHandle();

    TMsgPlayerHit msg;
    msg.h_sender = CHandle(this).getOwner();
    player->sendMsg(msg);
}

void TCompAIPatrol::playStepParticle(bool left)
{
    TCompGroup* my_group = get<TCompGroup>();
    if (my_group) {
        CHandle foot = left ? my_group->getHandleByName("left_foot") : my_group->getHandleByName("right_foot");
        EngineParticles.launchSystem("data/particles/def_amb_ground_slam.particles", foot);
    }
}

void TCompAIPatrol::shakeCamera(float max_amount, float max_distance, float duration)
{
    VHandles v_tp_cameras = CTagsManager::get().getAllEntitiesByTag(getID("tp_camera"));
    TCompTransform* my_pos = get<TCompTransform>();
    CEntity* e_player = EngineEntities.getPlayerHandle();
    TCompTransform* ppos = e_player->get<TCompTransform>();

    float distance = VEC3::Distance(ppos->getPosition(), my_pos->getPosition());
    distance = Clamp(distance, 0.f, max_distance);

    TMsgCameraShake msg;
    msg.amount = lerp(max_amount, 0.f, distance/ max_distance);
    msg.speed = 140.f;
    msg.time_to_stop = duration;
    for (int i = 0; i < v_tp_cameras.size(); i++) {
        v_tp_cameras[i].sendMsg(msg);
    }
}

void TCompAIPatrol::playSlamParticle()
{
    TCompGroup* my_group = get<TCompGroup>();
    CHandle foot = my_group->getHandleByName("right_foot");
    // Sacalo al json si te apetece.
    EngineParticles.launchSystem("data/particles/def_patrol_slam.particles", foot);
    EngineParticles.launchSystem("data/particles/def_patrol_slam_core.particles", foot);
    EngineParticles.launchSystem("data/particles/def_patrol_slam_sphere.particles", foot);

    EngineParticles.launchSystem("data/particles/def_patrol_slam_trails.particles", foot);
    EngineParticles.launchSystem("data/particles/def_patrol_slam_trails_core.particles", foot);
}

void TCompAIPatrol::playAnimationByName(const std::string & animationName)
{
    TCompPatrolAnimator * myAnimator = get<TCompPatrolAnimator>();
    myAnimator->playAnimationConverted(myAnimator->getAnimationByName(animationName));
}

void TCompAIPatrol::resetAnimationCompletedBooleans() {
	inhibitorAnimationCompleted = false;
	attackAnimationCompleted = false;
}
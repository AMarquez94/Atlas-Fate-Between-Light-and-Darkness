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
	//timerToExplode += dt;
	//TCompRigidbody* rigidbody = get<TCompRigidbody>();
	//TCompTransform* myTransform = get<TCompTransform>();

	//VEC3 direction = getWaypoint().position - myTransform->getPosition();
	//if (timerToExplode > 3) {
	//	direction.x = -direction.x;
	//}
	//direction.Normalize();
	//rigidbody->addForce(direction * force);
	//dbg("Force added (%f, %f, %f)\n", (direction * force).x, (direction * force).y, (direction * force).z);

	//if (timerToExplode > 6) {
	//	timerToExplode = 0;
	//}

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
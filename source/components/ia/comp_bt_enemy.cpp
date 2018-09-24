#include "mcv_platform.h"
#include "comp_bt_enemy.h"
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
#include "components/comp_tags.h"

void TCompAIEnemy::debugInMenu() {

    TCompIAController::debugInMenu();
    CEntity * me = myHandle.getOwner();

    if (current) {
        validState = current->getName();
    }

    if (navmeshPath.size() > 1) {
        for (int i = 0; i < navmeshPath.size() - 1; i++) {
            renderLine(navmeshPath[i], navmeshPath[i + 1], VEC4(1, 0, 0, 1));
        }
    }

    ImGui::Text("Current state: %s", validState.c_str());

    ImGui::Text("Suspect Level:");
    ImGui::SameLine();
    ImGui::ProgressBar(suspectO_Meter);

    ImGui::Text("Last player pos: %f, %f, %f", lastPlayerKnownPos.x, lastPlayerKnownPos.y, lastPlayerKnownPos.z);

    TCompTransform *tpos = me->get<TCompTransform>();
    ImGui::Text("My Pos: (%f, %f, %f)", tpos->getPosition().x, tpos->getPosition().y, tpos->getPosition().z);
    ImGui::Text(" Noise: (%f, %f, %f)", noiseSource.x, noiseSource.y, noiseSource.z);

    ImGui::Text("Time waited at unreachable point: %f", timerWaitingInUnreachablePoint);
    ImGui::Text("Navmesh size: %d - My point %d", navmeshPath.size(), navmeshPathPoint);
}

void TCompAIEnemy::getClosestWpt()
{
    float minDistance = INFINITY;
    int  minIndexWpt = 0;

    CEntity * me = myHandle.getOwner();
    TCompTransform *mypos = me->get<TCompTransform>();

    for (int i = 0; i < _waypoints.size(); i++) {
        float currDistance = VEC3::Distance(mypos->getPosition(), _waypoints[i].position);
        if (currDistance < minDistance) {
            minDistance = currDistance;
            minIndexWpt = i;
        }
    }

    currentWaypoint = minIndexWpt;
}

/* AUX FUNCTIONS */
bool TCompAIEnemy::rotateTowardsVec(VEC3 objective, float rotationSpeed, float dt) {
    CEntity * me = myHandle.getOwner();
    bool isInObjective = false;
    TCompTransform *mypos = me->get<TCompTransform>();
    float y, r, p;
    mypos->getYawPitchRoll(&y, &p, &r);

    float deltaYaw = mypos->getDeltaYawToAimTo(objective);
    if (fabsf(deltaYaw) <= rotationSpeed * dt) {
        y += deltaYaw;
        isInObjective = true;
    }
    else {
        if (mypos->isInLeft(objective))
        {
            y += rotationSpeed * dt;
        }
        else {
            y -= rotationSpeed * dt;
        }
    }
    mypos->setYawPitchRoll(y, p, r);
    return isInObjective;
}

bool TCompAIEnemy::isEntityInFov(const std::string& entityToChase, float fov, float maxChaseDistance) {
    CEntity * me = myHandle.getOwner();
    TCompTransform *mypos = me->get<TCompTransform>();
    TCompCollider *myCollider = me->get<TCompCollider>();
    CPhysicsCapsule * capsuleCollider = (CPhysicsCapsule *)myCollider->config;
    float myY = mypos->getPosition().y;

    CHandle hPlayer = EngineEntities.getPlayerHandle();
    if (hPlayer.isValid()) {
        CEntity *ePlayer = hPlayer;
        TCompTransform *ppos = ePlayer->get<TCompTransform>();
        float playerY = ppos->getPosition().y;
        float dist = VEC3::Distance(mypos->getPosition(), ppos->getPosition());
        TCompTempPlayerController *pController = ePlayer->get<TCompTempPlayerController>();

        /* Player inside cone of vision */
        bool in_fov = mypos->isInFov(ppos->getPosition(), fov, deg2rad(45.f))/* && fabsf(myY - playerY) <= 2 * capsuleCollider->height*/; //in fov and not too high

        //bool in_horizontal_fov = mypos->isInHorizontalFov(ppos->getPosition(), fov);
        //bool in_vertical_fov = mypos->isInVerticalFov(ppos->getPosition(), deg2rad(45.f));

        return in_fov && !pController->isInvisible && !pController->isInNoClipMode &&
            !pController->isMerged && !pController->isDead() && dist <= maxChaseDistance && !isEntityHidden(hPlayer);
    }
    else {
        return false;
    }
}

bool TCompAIEnemy::isEntityHidden(CHandle hEntity)
{
    CEntity * me = myHandle.getOwner();
    CEntity *entity = hEntity;
    TCompTransform *mypos = me->get<TCompTransform>();
    TCompTransform *eTransform = entity->get<TCompTransform>();
    TCompCollider *myCollider = me->get<TCompCollider>();
    TCompCollider *eCollider = entity->get<TCompCollider>();

    CPhysicsCapsule * capsuleCollider = (CPhysicsCapsule *)myCollider->config;

    bool isHidden = true;

    VEC3 myPosition = mypos->getPosition();
    VEC3 origin = myPosition + VEC3(0, capsuleCollider->height * 2, 0);
    VEC3 dest = VEC3::Zero;
    VEC3 dir = VEC3::Zero;

    float i = 0;
    while (isHidden && i < capsuleCollider->height * 2) {
        dest = eTransform->getPosition() + VEC3(0, Clamp(i - .1f, 0.f, capsuleCollider->height * 2), 0);
        dir = dest - origin;
        dir.Normalize();
        physx::PxRaycastHit hit;
        float dist = VEC3::Distance(origin, dest);

        //TODO: only works when behind scenery. Make the same for other enemies, dynamic objects...
        if (!EnginePhysics.Raycast(origin, dir, dist, hit, physx::PxQueryFlag::eSTATIC)) {
            isHidden = false;
        }
        i = i + (capsuleCollider->height / 2);
    }
    return isHidden;
}

void TCompAIEnemy::generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc)
{
    navmeshPath = EngineNavmeshes.findPath(initPos, destPos);
    navmeshPathPoint = 0;
    recalculateNavmesh = recalc;

    isDestinationCloseEnough = EngineNavmeshes.navmeshLong(navmeshPath) < maxNavmeshDistance;

    /* Calculate navmesh generation */
    //
    //if (navmeshPath.size() == 0) {
    //    navmeshPath.push_back(initPos);
    //}

    if (navmeshPath.size() > 0) {
        VEC3 lastNavmeshPoint = navmeshPath[navmeshPath.size() - 1];
        float diff = VEC3::Distance(destPos, lastNavmeshPoint);
        float diffY = fabsf(destPos.y - lastNavmeshPoint.y);
        //dbg("Can arrive to destination by its long %s\n", diff < 1.5f ? "YES" : "NO");
        //dbg("Can arrive to destination by its height %s\n", diffY < 0.3f ? "YES" : "NO");
        canArriveToDestination = diff < 1.5f && diffY < 0.3f;
    }
    else {
        float diff = VEC3::Distance(initPos, destPos);
        float diffY = fabsf(initPos.y - destPos.y);

        std::vector<physx::PxSweepHit> hits;
        CEntity * me = myHandle.getOwner();
        TCompTransform *mypos = me->get<TCompTransform>();
        TCompCollider* mycol = me->get<TCompCollider>();

        VEC3 offsetY = VEC3::Zero;
        physx::PxGeometryHolder geometryHolder = mycol->config->getGeometry();

        switch (mycol->config->getGeometry().getType()) {
        case physx::PxGeometryType::eCAPSULE:
            offsetY = VEC3(0.f, geometryHolder.capsule().halfHeight + 0.1f, 0.f);
            break;
        case physx::PxGeometryType::eBOX:
            offsetY = VEC3(0.f, geometryHolder.box().halfExtents.y + 0.1f, 0.f);
            break;
        default:
            break;
        }

        // Set a default filter to do query checks
        physx::PxFilterData pxFilterData;
        pxFilterData.word0 = FilterGroup::All;
        physx::PxQueryFilterData defaultFilter;
        defaultFilter.data = pxFilterData;

        bool sweephit = EnginePhysics.Sweep(mycol->config->getGeometry().any(), initPos + offsetY, mypos->getRotation(), (destPos - initPos).Normalized(), diff, hits, (physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC), defaultFilter);
        //dbg("Can arrive to destination by its sweep - Zero navmesh %s\n", !sweephit ? "YES" : "NO");
        //dbg("Can arrive to destination by its height - Zero navmesh %s\n", diffY < 0.3f ? "YES" : "NO");

        bool realHit = false;

        if (sweephit) {
            for (int i = 0; i < hits.size() && !realHit; i++) {
                CHandle h;
                h.fromVoidPtr(hits[i].actor->userData);
                TCompCollider * othercol = h;
                h = h.getOwner();
                CEntity* e = h;
                TCompTags* otherTags = e->get<TCompTags>();
                if (otherTags && !otherTags->hasTag(getID("enemy")) && !otherTags->hasTag(getID("player"))) {
                    realHit = hits[i].distance < diff;
                }
                //dbg("Hit with %s\n", e->getName());
            }
        }

        canArriveToDestination = diff < 2.f && !realHit && diffY < 0.3f;
        if (!canArriveToDestination) {
            navmeshPath.push_back(initPos);
        }
        else {
            navmeshPath.push_back(destPos);
        }
    }
    timerWaitingInUnreachablePoint = 0;
}

bool TCompAIEnemy::moveToPoint(float speed, float rotationSpeed, VEC3 objective, float dt)
{
    CEntity * me = myHandle.getOwner();
    TCompTransform *mypos = me->get<TCompTransform>();

    VEC3 nextPos = navmeshPath.size() > 0 && navmeshPathPoint < navmeshPath.size() ?
        navmeshPath[navmeshPathPoint] :
        objective;

    rotateTowardsVec(nextPos, rotationSpeed, dt);

    VEC3 left = mypos->getLeft();
    left.Normalize();
    VEC3 finalDir = objective - mypos->getPosition();
    finalDir = VEC3(finalDir.x, 0.f, finalDir.z);
    finalDir.Normalize();
    VEC3 intermediateDir = nextPos - mypos->getPosition();
    intermediateDir = VEC3(intermediateDir.x, 0, intermediateDir.z);
    intermediateDir.Normalize();

    VEC3 vp = mypos->getPosition();

    if (VEC3::Distance(objective, vp) <= speed * dt + 0.1f/*fabsf(left.Dot(finalDir)) * maxDistanceToNavmeshPoint + 0.1f*/) {
        return true;
    }
    else {
        float actualSpeed = speed;
        VEC3 front = mypos->getFront();
        front.Normalize();
        if (fabsf(front.Dot(intermediateDir) < 0.6f)) {
            actualSpeed = 0;
        }
        else if (!recalculateNavmesh) {
            generateNavmesh(vp, objective, true);
        }
        vp = vp + actualSpeed * dt * front;
        mypos->setPosition(vp);				//Move towards wpt

        if (VEC3::Distance2D(nextPos, vp) <= fabsf(left.Dot(intermediateDir) * maxDistanceToNavmeshPoint) + 0.1f) {
            navmeshPathPoint++;
        }
        return false;
    }
}

bool TCompAIEnemy::isCurrentDestinationReachable()
{
    return canArriveToDestination && isDestinationCloseEnough;
}

void TCompAIEnemy::sendSuspectingMsg(bool isSuspecting)
{
    if (isSuspecting != isSuspectingAndSeeingPlayer) {
        CEntity* e_player = EngineEntities.getPlayerHandle();
        TMsgEnemySuspecting msg;
        msg.enemy_suspecting = myHandle.getOwner();
        msg.is_suspecting = isSuspecting;
        e_player->sendMsg(msg);
        isSuspectingAndSeeingPlayer = isSuspecting;
    }
}

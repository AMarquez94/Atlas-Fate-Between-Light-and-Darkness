#pragma once

#include "components/comp_base.h"
#include "comp_bt.h"
#include "modules/module_ia.h"

class TCompAIEnemy : public TCompIAController {

protected:

    struct StateColors {
        VEC4 colorNormal;
        VEC4 colorSuspect;
        VEC4 colorAlert;
        VEC4 colorDead;
    } enemyColor;

    /* Atributes */
    std::vector<Waypoint> _waypoints;
    int currentWaypoint;

    /* Suspect */
    float suspectO_Meter = 0.f;
    bool isSuspectingAndSeeingPlayer = false;
    bool isLastPlayerKnownDirLeft = false;
    VEC3 lastPlayerKnownPos = VEC3::Zero;
    bool alarmEnded = true;
    bool hasBeenStunned = false;

    /* Noise management */
    bool hasHeardNaturalNoise = false;
    bool hasHeardArtificialNoise = false;
    VEC3 noiseSource = VEC3::Zero;
    bool noiseSourceChanged = false;
    CHandle hNoiseSource = CHandle();
    std::chrono::steady_clock::time_point lastTimeNoiseWasHeard;

    /* Pushed */
    VEC3 pushedDirection = VEC3::Zero;
    float pushedTime = 0.f;
    bool isPushing = false;

    /* Vision */
    physx::PxQueryFilterData visionFilter;

    std::string validState = "";

    /* Navmesh management */
    std::vector<VEC3> navmeshPath;
    int navmeshPathPoint = 0;
    bool recalculateNavmesh = false;
    float maxDistanceToNavmeshPoint = 3.f;
    bool canArriveToDestination = true;
    float maxNavmeshDistance = 60.f;    //TODO: Probar distintos atributos
    bool isDestinationCloseEnough = true;

    /* Timers */
    float timerWaitingInWpt = 0.f;
    float timerWaitingInNoise = 0.f;
    float timerWaitingInUnreachablePoint = 0.f;

    /* Aux functions */
    void onCreate();
    const Waypoint getWaypoint() { return _waypoints[currentWaypoint]; }
    void addWaypoint(const Waypoint& wpt) { _waypoints.push_back(wpt); };
    void getClosestWpt();
    bool rotateTowardsVec(VEC3 objective, float rotationSpeed, float dt);
    bool isEntityInFov(const std::string& entityToChase, float fov, float maxChaseDistance);
    bool isEntityHidden(CHandle hEntity);
    void generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc = false);
    bool moveToPoint(float speed, float rotationSpeed, VEC3 destPoint, float dt);
    bool isCurrentDestinationReachable();
    void sendSuspectingMsg(bool isSuspecting);
    void sendNothingHereMsg();

    /* TODO: Delete */
    float maxDiff = 0;
    float maxYDiff = 0;

public:

    ~TCompAIEnemy();

    void debugInMenu();

    bool isStunned() { return current && current->getName().compare("stunned") == 0; }

    virtual const std::string getStateForCheckpoint() = 0;
    virtual void playAnimationByName(const std::string& animationName) = 0;

    float getSuspectLevel() { return suspectO_Meter; };
};
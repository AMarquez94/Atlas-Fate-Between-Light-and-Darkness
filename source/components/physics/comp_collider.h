#pragma once

#include "components/comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"

class CPhysicsCollider;
class TCompTransform;

class TCompCollider : public TCompBase {

    std::string shapeName = "";
    std::string groupName = "";
    std::string maskName = "";

    void createCollider();

    void onCreate(const TMsgEntityCreated& msg);
    void onGroupCreated(const TMsgEntitiesGroupCreated& msg);
    void onDestroy(const TMsgEntityDestroyed& msg);
    void onTriggerEnter(const TMsgTriggerEnter& msg);
    void onTriggerExit(const TMsgTriggerExit& msg);

    DECL_SIBLING_ACCESS();

public:

    bool player_inside;
    bool player_full_inside;
    std::map<uint32_t, TCompTransform*> handles;

    // Collider parameter description
    CPhysicsCollider * config;

    ~TCompCollider();
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    void renderDebug();

    /* Auxiliar methods */
    bool collisionDistance(const VEC3 & org, const VEC3 & dir, float maxDistance);
    void setGlobalPose(VEC3 newPos, VEC4 newRotation, bool autowake = false);

    //void enableCollisionsAndQueries(bool disable);    /* To be tested (and understood :')) */

    static void registerMsgs();
};
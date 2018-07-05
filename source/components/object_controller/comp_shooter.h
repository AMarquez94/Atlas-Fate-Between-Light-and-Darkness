#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompShooter : public TCompBase {

    enum EShooterState {
        STOPPED,
        FIRING
    };

    EShooterState state;

    CHandle h_entity_to_shoot;
    float precission;
    float firing_rate_cooldown;
    float bullet_damage;
    float bullet_speed;
    float bullet_range;

    physx::PxQueryFilterData pxQueryFilterData;

    float firing_timer = 0.f;

    DECL_SIBLING_ACCESS();

    void shoot();
    void onScenePaused(const TMsgScenePaused& msg);

public:

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    static void registerMsgs();
    void setIsFiring(bool isFiring, CHandle h_entityToFire);
};
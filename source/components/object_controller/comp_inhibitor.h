#pragma once

#include "components/comp_base.h"
#include "entity/common_msgs.h"

class TCompInhibitor : public TCompBase {

    DECL_SIBLING_ACCESS();

    CHandle h_parent;
    float speed;
    float fading_speed;
    float initial_intensity;

    VEC3 dest;
    bool exploding = false;
    bool fading = false;

    bool playerWasInhibited = false;
    /* TODO: bool playerWasInhibited = false;
    float explosionSpeed;
    float explosionRange;*/

    void onMsgEntityCreated(const TMsgEntityCreated& msg);

public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    static void registerMsgs();
};
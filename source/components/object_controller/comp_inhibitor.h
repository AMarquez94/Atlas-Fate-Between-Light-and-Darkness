#pragma once

#include "components/comp_base.h"
#include "entity/common_msgs.h"

class TCompInhibitor : public TCompBase {

    DECL_SIBLING_ACCESS();

    CHandle h_parent;
    float min_x_speed;
    VEC2 time_range;
    float time;
    VEC3 dest;
    float speed;
    float currentTime;

    void onMsgEntityCreated(const TMsgEntityCreated& msg);
    void onMsgLaunchInhibitor(const TMsgLaunchInhibitor& msg);

public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    static void registerMsgs();
};
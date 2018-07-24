#pragma once

#include "components/comp_base.h"
#include "entity/common_msgs.h"

class TCompInhibitor : public TCompBase {

    DECL_SIBLING_ACCESS();

    CHandle h_parent;
    float speed;
    VEC3 dest;
    bool exploding = false;

    void onMsgEntityCreated(const TMsgEntityCreated& msg);

public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    static void registerMsgs();
};
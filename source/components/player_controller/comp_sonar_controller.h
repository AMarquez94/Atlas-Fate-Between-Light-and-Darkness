#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompSonarController : public TCompBase {

    void onSceneCreated(const TMsgSceneCreated& msg);
    void onPlayerExposed(const TMsgPlayerIlluminated& msg);

    DECL_SIBLING_ACCESS();
public:

    float total_time;
    float time_elapsed;

    void Init();
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    static void registerMsgs();
};
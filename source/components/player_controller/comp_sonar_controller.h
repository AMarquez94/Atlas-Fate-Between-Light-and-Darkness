#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompSonarController : public TCompBase {

    void onSonarActive(const TMsgSonarActive & msg);

    CTimer time_controller;
    std::string target_tag;
    std::vector<CHandle> target_handles;

    DECL_SIBLING_ACCESS();
public:

    float total_time;
    float alpha_value;
    float cooldown_time;

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    const bool canDeploySonar();

    static void registerMsgs();
};
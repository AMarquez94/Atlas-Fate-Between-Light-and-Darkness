#pragma once

#include "components/comp_base.h"

struct TCompSuspectElement : public TCompBase {

private:

    DECL_SIBLING_ACCESS();

    float offset_dist;
    CHandle parent;
    CHandle h_suspecting;
    bool active;

    void onEnemySuspecting(const TMsgEnemySuspecting & msg);

public:

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    static void registerMsgs();
};

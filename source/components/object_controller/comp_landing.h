#pragma once

#include "components/comp_base.h"

struct TCompLanding : public TCompBase {

private:

    DECL_SIBLING_ACCESS();

    std::string parentName;
    CHandle h_parent;
    bool is_active;

    void onMsgGroupCreated(const TMsgEntitiesGroupCreated& msg);
    void onMsgEntityCanLandSM(const TMsgEntityCanLandSM& msg);

public:

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    static void registerMsgs();

    const bool isActive() { return is_active; };
};

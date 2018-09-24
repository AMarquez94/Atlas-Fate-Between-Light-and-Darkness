#pragma once

#include "components/comp_base.h"

class TCompSuspectManager : public TCompBase {

    struct SuspectElement {
        CHandle enemySuspecting;
        CHandle element;
    };

    DECL_SIBLING_ACCESS();

    std::vector<SuspectElement> suspectElements;
    void onEnemySuspecting(const TMsgEnemySuspecting & msg);

    void createSuspectElement(CHandle enemySuspecting);

public:

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    static void registerMsgs();
};
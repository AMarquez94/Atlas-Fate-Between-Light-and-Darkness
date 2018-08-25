#pragma once

#include "components/comp_base.h"

class TCompBillBoard : public TCompBase
{
    DECL_SIBLING_ACCESS();

public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    static void registerMsgs();

};


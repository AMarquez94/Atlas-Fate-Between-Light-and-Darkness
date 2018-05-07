#pragma once

#include "components/comp_base.h"

class TCompRotator : public TCompBase
{
    float _speed;

    DECL_SIBLING_ACCESS();

public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    static void registerMsgs();

};


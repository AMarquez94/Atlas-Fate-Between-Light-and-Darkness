#pragma once

#include "components/comp_base.h"

class TCompRotator : public TCompBase
{
    float _speed;
    float _acceleration;
    float _total_accel = 0.f;
    float _max_acceleration;

    bool _rotate_pitch = false;
	bool _rotate_roll = false;

    DECL_SIBLING_ACCESS();

public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    static void registerMsgs();

};


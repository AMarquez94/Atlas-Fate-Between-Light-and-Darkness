#pragma once

#include "components\comp_base.h"

class TCompDynamicCapsule : public TCompBase {

    DECL_SIBLING_ACCESS();

    float speed;
    float elapsed_time;
    VEC3 start_point;
    VEC3 end_point;
    VEC3 offset;
    VEC3 director;

    void onMsgScenePaused(const TMsgScenePaused& msg);

public:

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    void setSpeed(float newSpeed);
    void setStartPoint(VEC3 newStartPoint);
    void setEndPoint(VEC3 newEndPoint);
    void setOffset(VEC3 newOffset);
    static void registerMsgs();
};

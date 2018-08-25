#pragma once

#include "components/comp_base.h"

class TCompBoneController : public TCompBase
{
    std::string _bone;
    std::string _target;

    CHandle h_target;
    TCompSkeleton * target_skeleton;

    void onSceneCreated(const TMsgSceneCreated& msg);

    DECL_SIBLING_ACCESS();

public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    static void registerMsgs();

};


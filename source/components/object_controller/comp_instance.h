#pragma once

#include "components/comp_base.h"

class TInstance;

class TCompInstance : public TCompBase
{
    int _index;
    std::string _instance_mesh;

    DECL_SIBLING_ACCESS();

public:
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    void onMsgEntityCreated(const TMsgEntityCreated& msg);
    void onGroupCreated(const TMsgEntitiesGroupCreated& msg);
    static void registerMsgs();

};


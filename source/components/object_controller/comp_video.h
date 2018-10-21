#pragma once

#include "components/comp_base.h"

class CVideoTexture;

class TCompVideo : public TCompBase
{
    CVideoTexture * _video;

    DECL_SIBLING_ACCESS();

public:
    bool enabled;

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    static void registerMsgs();

    void onMsgVideoStatus(const TMsgVideoStatus & msg);
    void onMsgEntityCreated(const TMsgEntityCreated& msg);
};


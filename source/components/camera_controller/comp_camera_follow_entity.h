#pragma once

#include "components/comp_base.h"
#include "camera/camera.h"
#include "entity/common_msgs.h"

class TCompCameraFollowEntity : public CCamera, public TCompBase
{
private:
    CHandle     _h_target;
    std::string _target_name;
    VEC3 offset;

    bool active;

    void onPause(const TMsgScenePaused& msg);

    DECL_SIBLING_ACCESS();

    void onMsgCameraActive(const TMsgCameraActivated &msg);
    void onMsgCameraFullActive(const TMsgCameraFullyActivated &msg);
    void onMsgCameraDeprecated(const TMsgCameraDeprecated &msg);
    void onMsgCameraSetActive(const TMsgSetCameraActive &msg);


public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    static void registerMsgs();
};
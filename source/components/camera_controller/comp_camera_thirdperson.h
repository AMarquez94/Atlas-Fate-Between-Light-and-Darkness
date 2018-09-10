#pragma once

#include "components/comp_base.h"
#include "camera/camera.h"
#include "entity/common_msgs.h"

class TCompCameraThirdPerson : public CCamera, public TCompBase
{
private:
    CHandle     _h_target;
    std::string _target_name;

    float _speed;
    VEC2 _clamp_angle;
    VEC2 _current_euler;
    VEC2 _original_euler;
    VEC3 _clipping_offset;

    float _timer_fov = 0.f;
    float _max_time_fov = 1.f;
    float _target_fov = 70.f;

    bool active;

    const Input::TButton& btHorizontal = EngineInput["MouseX"];
    const Input::TButton& btVertical = EngineInput["MouseY"];
    const Input::TButton& btDebugPause = EngineInput["btDebugPause"];
    void onPause(const TMsgScenePaused& msg);

    DECL_SIBLING_ACCESS();

    void onPause(const TMsgScenePaused& msg);
    void onMsgCameraActive(const TMsgCameraActivated &msg);
    void onMsgCameraFullActive(const TMsgCameraFullyActivated &msg);
    void onMsgCameraDeprecated(const TMsgCameraDeprecated &msg);
    void onMsgCameraSetActive(const TMsgSetCameraActive &msg);
    void onMsgCameraReset(const TMsgCameraReset &msg);
    void onMsgCameraResetTargetPos(const TMsgCameraResetTargetPos &msg);
    void onMsgCameraFov(const TMsgCameraFov &msg);

    float getFovUpdated(float dt);


public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    const VEC2 getCurrentEuler() { return _current_euler; }
    float CameraClipping(const VEC3 & origin, const VEC3 & dir);
    void setCurrentEuler(float euler_x = INFINITY, float euler_y = INFINITY);
    void resetCamera(bool both_angles, bool only_y);
    void resetCameraTargetPos();

    static void registerMsgs();
};
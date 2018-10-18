#pragma once

#include "components/comp_base.h"
#include "camera/camera.h"
#include "entity/common_msgs.h"

class TCompCameraShadowMerge : public CCamera, public TCompBase
{
private:
    CHandle     _h_target;
    std::string _target_name;
    physx::PxQueryFilterData cameraFilter;

    float _speed;
    VEC2 _clamp_angle;
    VEC2 _original_euler;
    VEC2 _current_euler;
    VEC3 _clipping_offset;
    float _starting_pitch;

    const Input::TButton& btHorizontal = EngineInput["Horizontal"];
    const Input::TButton& btVertical = EngineInput["Vertical"];
    const Input::TButton& btRHorizontal = EngineInput["MouseX"];
    const Input::TButton& btRVertical = EngineInput["MouseY"];

    bool active;

    void onMsgCameraActive(const TMsgCameraActivated &msg);
    void onMsgCameraFullActive(const TMsgCameraFullyActivated &msg);
    void onMsgCameraDeprecated(const TMsgCameraDeprecated &msg);
    void onMsgActivateMyself(const TMsgSetCameraActive &msg);
    void onMsgScenePaused(const TMsgScenePaused &msg);
    void onMsgCameraReset(const TMsgCameraReset &msg);
    void onMsgCameraResetTargetPos(const TMsgCameraResetTargetPos& msg);

    void activateCamera();

public:
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    float CameraClipping(const VEC3 & origin, const VEC3 & dir);

    DECL_SIBLING_ACCESS();

    static void registerMsgs();

    const float getSpeed() { return _speed; }
    const VEC2 getClampAngle() { return _clamp_angle; }
    const VEC2 getOriginalEuler() { return _original_euler; }
    const VEC2 getCurrentEuler() { return _current_euler; }
    const VEC3 getClippingOffset() { return _clipping_offset; }
    const CHandle getCameraTarget() { return _h_target; }
    const float getStartingPitch() { return _starting_pitch; };
    void resetCameraTargetPos();

    void setCurrentEuler(float x = INFINITY, float y = INFINITY);
};
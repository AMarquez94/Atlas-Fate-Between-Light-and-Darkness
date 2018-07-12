#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_camera_shadowmerge.h"
#include "components/camera_controller/comp_camera_thirdperson.h"
#include "components/comp_transform.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("camera_shadowmerge", TCompCameraShadowMerge);

// Same as the thirdperson camera by now
// This will need to be changed for vertical shadow merging.

void TCompCameraShadowMerge::debugInMenu()
{
    ImGui::DragFloat3("Offsets", &_clipping_offset.x, 0.1f, -20.f, 20.f);
    ImGui::DragFloat2("Angles", &_clamp_angle.x, 0.1f, -90.f, 90.f);
    ImGui::DragFloat("Speed", &_speed, 0.1f, 0.f, 20.f);
}

void TCompCameraShadowMerge::load(const json& j, TEntityParseContext& ctx)
{
    // Read from the json all the input data
    _speed = j.value("speed", 1.5f);
    _target_name = j.value("target", "");
    _clamp_angle = loadVEC2(j["clampangle"]);
    _clipping_offset = loadVEC3(j["offset"]);
    _clamp_angle = VEC2(deg2rad(_clamp_angle.x), deg2rad(_clamp_angle.y));
    _starting_pitch = deg2rad(j.value("starting_pitch", 0.f));

    // Load the target and set his axis as our axis.
    _h_target = ctx.findEntityByName(_target_name);
    TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>();

    float yaw, pitch, roll;
    target_transform->getYawPitchRoll(&yaw, &pitch, &roll);
    _original_euler = VEC2(yaw, pitch + _starting_pitch);
    _current_euler = _original_euler;

    active = false;

    physx::PxFilterData pxFilterData;
    pxFilterData.word1 = FilterGroup::Scenario;

    cameraFilter.data = pxFilterData;
}

void TCompCameraShadowMerge::registerMsgs()
{
    DECL_MSG(TCompCameraShadowMerge, TMsgCameraActivated, onMsgCameraActive);
    DECL_MSG(TCompCameraShadowMerge, TMsgCameraFullyActivated, onMsgCameraFullActive);
    DECL_MSG(TCompCameraShadowMerge, TMsgCameraDeprecated, onMsgCameraDeprecated);
    DECL_MSG(TCompCameraShadowMerge, TMsgSetCameraActive, onMsgActivateMyself);
    DECL_MSG(TCompCameraShadowMerge, TMsgScenePaused, onMsgScenePaused);
    DECL_MSG(TCompCameraShadowMerge, TMsgCameraReset, onMsgCameraReset);
}

void TCompCameraShadowMerge::onMsgCameraActive(const TMsgCameraActivated &msg)
{
    CEntity * eCamera = msg.previousCamera;
    TCompCameraThirdPerson * c_camera = eCamera->get<TCompCameraThirdPerson>();

    if (c_camera) {
        /* Reset y angle in all tp_cameras */
        VHandles v_tp_cameras = CTagsManager::get().getAllEntitiesByTag(getID("sm_camera"));
        TMsgCameraReset msg;
        msg.both_angles = false;
        msg.only_y = true;
        for (int i = 0; i < v_tp_cameras.size(); i++) {
            v_tp_cameras[i].sendMsg(msg);
        }
    }
}

void TCompCameraShadowMerge::onMsgCameraFullActive(const TMsgCameraFullyActivated & msg)
{
    active = true;
    //dbg("Camera full active %s\n", ((TCompName*)get<TCompName>())->getName());
}

void TCompCameraShadowMerge::onMsgCameraDeprecated(const TMsgCameraDeprecated &msg)
{
    active = false;
    TCompCameraThirdPerson *tTpCamera = ((CEntity*)getEntityByName("TPCamera"))->get<TCompCameraThirdPerson>();
    _current_euler.x = tTpCamera->getCurrentEuler().x;
    //_cur
    //_current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, _original_euler.y);
    //dbg("Camera inactive %s\n", ((TCompName*)get<TCompName>())->getName());
}

void TCompCameraShadowMerge::onMsgActivateMyself(const TMsgSetCameraActive & msg)
{
    if (active) {
        Engine.getCameras().cancelCamera(CHandle(this).getOwner());
    }

    Engine.getCameras().blendInCamera(CHandle(this).getOwner(), .2f, CModuleCameras::EPriority::GAMEPLAY);
}

void TCompCameraShadowMerge::onMsgScenePaused(const TMsgScenePaused & msg)
{
    paused = msg.isPaused;
}

void TCompCameraShadowMerge::onMsgCameraReset(const TMsgCameraReset & msg)
{
    if (msg.both_angles) {
        //_current_euler = _original_euler;
        _current_euler.x = _original_euler.x;
        _current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, -_clamp_angle.x);
    }
    else if (msg.only_y) {
        //_current_euler.y = _original_euler.y;
        _current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, -_clamp_angle.x);
    }
    else {
        _current_euler.x = _original_euler.x;
    }
}

void TCompCameraShadowMerge::update(float dt)
{
    if (!paused) {
        if (!_h_target.isValid())
            return;

        TCompTransform* self_transform = get<TCompTransform>();
        TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>(); // we will need to consume this.
        assert(self_transform);
        assert(target_transform);

        float horizontal_delta = 0.f;
        float vertical_delta = 0.f;


        // To remove in the future.
        horizontal_delta = EngineInput.mouse()._position_delta.x;
        vertical_delta = -EngineInput.mouse()._position_delta.y;
        if (btRHorizontal.isPressed()) horizontal_delta = btRHorizontal.value;
        if (btRVertical.isPressed()) vertical_delta = btRVertical.value;

        // Verbose code
        _current_euler.x -= horizontal_delta * _speed * dt;

        //if (active) {
            _current_euler.y += vertical_delta * _speed * dt;
            _current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, -_clamp_angle.x);
        //}

        // EulerAngles method based on mcv class
        VEC3 vertical_offset = 0.1f * target_transform->getUp() * _clipping_offset.y; // Change VEC3::up, for the players vertical angle, (TARGET VERTICAL)
        VEC3 horizontal_offset = self_transform->getLeft() * _clipping_offset.x;
        VEC3 target_position = target_transform->getPosition() + vertical_offset + horizontal_offset;

        //self_transform->setPosition(target_position);
        self_transform->setYawPitchRoll(_current_euler.x, _current_euler.y, 0);

        float z_distance = CameraClipping(target_position, -self_transform->getFront());
        VEC3 new_pos = target_position + z_distance * -self_transform->getFront();
        self_transform->setPosition(new_pos);

        float inputSpeed = Clamp(fabs(btHorizontal.value) + fabs(btVertical.value), 0.f, 1.f);
        float current_fov = 70 + inputSpeed * 30; // Just doing some testing with the fov and speed
        setPerspective(deg2rad(current_fov), 0.1f, 1000.f);
    }
}

float TCompCameraShadowMerge::CameraClipping(const VEC3 & origin, const VEC3 & dir)
{
    physx::PxRaycastHit hit;
    if (EnginePhysics.Raycast(origin, dir, _clipping_offset.z, hit, physx::PxQueryFlag::eSTATIC, cameraFilter))
        return Clamp(hit.distance - 0.3f, 0.f, _clipping_offset.z);

    return _clipping_offset.z;
}

void TCompCameraShadowMerge::setCurrentEuler(float x, float y)
{
    if (x != INFINITY) _current_euler.x = x;
    if (y != INFINITY) _current_euler.y = y;
}

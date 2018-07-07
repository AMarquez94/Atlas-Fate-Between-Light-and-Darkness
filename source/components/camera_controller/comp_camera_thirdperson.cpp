#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_camera_thirdperson.h"
#include "components/comp_transform.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("camera_thirdperson", TCompCameraThirdPerson);
const Input::TInterface_Mouse& mouse = EngineInput.mouse();

void TCompCameraThirdPerson::debugInMenu()
{
    ImGui::DragFloat3("Offsets", &_clipping_offset.x, 0.1f, -20.f, 20.f);
    ImGui::DragFloat2("Angles", &_clamp_angle.x, 0.1f, -90.f, 90.f);
    ImGui::DragFloat("Speed", &_speed, 0.1f, 0.f, 20.f);
    ImGui::Text("Current euler %f - %f", rad2deg(_current_euler.x), rad2deg(_current_euler.y));
}

void TCompCameraThirdPerson::load(const json& j, TEntityParseContext& ctx)
{
    // Read from the json all the input data
    _speed = j.value("speed", 1.5f);
    _target_name = j.value("target", "");
    _clamp_angle = loadVEC2(j["clampangle"]);
    _clipping_offset = loadVEC3(j["offset"]);
    _clamp_angle = VEC2(deg2rad(_clamp_angle.x), deg2rad(_clamp_angle.y));

    // Load the target and set his axis as our axis.
    _h_target = ctx.findEntityByName(_target_name);
    TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>();

    float yaw, pitch, roll;
    target_transform->getYawPitchRoll(&yaw, &pitch, &roll);
    _current_euler = VEC2(yaw, pitch);
    _original_euler = _current_euler;

    active = false;
}

void TCompCameraThirdPerson::registerMsgs()
{
    DECL_MSG(TCompCameraThirdPerson, TMsgCameraActivated, onMsgCameraActive);
    DECL_MSG(TCompCameraThirdPerson, TMsgCameraDeprecated, onMsgCameraDeprecated);
    DECL_MSG(TCompCameraThirdPerson, TMsgCameraFullyActivated, onMsgCameraFullActive);
    DECL_MSG(TCompCameraThirdPerson, TMsgSetCameraActive, onMsgCameraSetActive);
    DECL_MSG(TCompCameraThirdPerson, TMsgScenePaused, onPause);
    DECL_MSG(TCompCameraThirdPerson, TMsgCameraReset, onMsgCameraReset);
}

void TCompCameraThirdPerson::onMsgCameraActive(const TMsgCameraActivated & msg)
{
    CEntity * eCamera = msg.previousCamera;
    TCompCameraThirdPerson * c_camera = eCamera->get<TCompCameraThirdPerson>();

    if (c_camera == nullptr) {
        /* Reset y angle in all tp_cameras */
        VHandles v_tp_cameras = CTagsManager::get().getAllEntitiesByTag(getID("tp_camera"));
        TMsgCameraReset msg;
        msg.both_angles = false;
        msg.only_y = true;
        for (int i = 0; i < v_tp_cameras.size(); i++) {
            v_tp_cameras[i].sendMsg(msg);
        }
    }
}

void TCompCameraThirdPerson::onMsgCameraFullActive(const TMsgCameraFullyActivated & msg)
{
    active = true;
}

void TCompCameraThirdPerson::onMsgCameraDeprecated(const TMsgCameraDeprecated & msg)
{
    active = false;
}

void TCompCameraThirdPerson::onMsgCameraSetActive(const TMsgSetCameraActive & msg)
{
    if (active) {
        Engine.getCameras().cancelCamera(CHandle(this).getOwner());
    }

    Engine.getCameras().blendInCamera(CHandle(this).getOwner(), .2f, CModuleCameras::EPriority::GAMEPLAY);
}

void TCompCameraThirdPerson::onMsgCameraReset(const TMsgCameraReset & msg)
{
    if (msg.both_angles) {
        //_current_euler = _original_euler;
        _current_euler.x = _original_euler.x;
        _current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, _original_euler.y);
    }
    else if (msg.only_y) {
        //_current_euler.y = _original_euler.y;
        _current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, _original_euler.y);
    }
    else {
        _current_euler.x = _original_euler.x;
    }
}

void TCompCameraThirdPerson::update(float dt)
{
    if (!paused) {

        if (!_h_target.isValid()) return;

        TCompTransform* self_transform = get<TCompTransform>();
        TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>(); // we will need to consume this.
        assert(self_transform);
        assert(target_transform);

        // To remove in the future.
        float horizontal_delta = mouse._position_delta.x;
        float vertical_delta = -mouse._position_delta.y;
        if (EngineInput["MouseX"].isPressed()) horizontal_delta = EngineInput["MouseX"].value;
        if (EngineInput["MouseY"].isPressed()) vertical_delta = EngineInput["MouseY"].value;

        // Verbose code
        _current_euler.x -= horizontal_delta * _speed * dt;
        _current_euler.y += vertical_delta * _speed * dt;
        _current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, -_clamp_angle.x);

        // EulerAngles method based on mcv class
        VEC3 vertical_offset = VEC3::Up * _clipping_offset.y; // Change VEC3::up, for the players vertical angle, (TARGET VERTICAL)
        VEC3 horizontal_offset = self_transform->getLeft() * _clipping_offset.x;
        VEC3 target_position = target_transform->getPosition() + vertical_offset + horizontal_offset;
        self_transform->setYawPitchRoll(_current_euler.x, _current_euler.y, 0);

        float z_distance = CameraClipping(target_position, -self_transform->getFront());
        VEC3 new_pos = target_position + z_distance * -self_transform->getFront();
        self_transform->setPosition(new_pos);

        float inputSpeed = Clamp(fabs(btHorizontal.value) + fabs(btVertical.value), 0.f, 1.f);
        float current_fov = 70 + inputSpeed * 30; // Just doing some testing with the fov and speed
        setPerspective(deg2rad(current_fov), 0.1f, 1000.f);
    }
}

float TCompCameraThirdPerson::CameraClipping(const VEC3	& origin, const VEC3 & dir)
{
    physx::PxRaycastHit hit;
    if (EnginePhysics.Raycast(origin, dir, _clipping_offset.z, hit, physx::PxQueryFlag::eSTATIC))
        return Clamp(hit.distance - 0.1f, 0.2f, _clipping_offset.z);

    return _clipping_offset.z;
}

void TCompCameraThirdPerson::setCurrentEuler(float euler_x, float euler_y)
{
    if (euler_x != INFINITY) _current_euler.x = euler_x;
    if (euler_y != INFINITY) _current_euler.y = euler_y;
}

void TCompCameraThirdPerson::onPause(const TMsgScenePaused& msg) {

    paused = msg.isPaused;
}


#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_camera_thirdperson.h"
#include "components/comp_transform.h"
#include "components/comp_tags.h"
#include "components/ia/comp_bt_player.h"

DECL_OBJ_MANAGER("camera_thirdperson", TCompCameraThirdPerson);
const Input::TInterface_Mouse& mouse = EngineInput.mouse();

void TCompCameraThirdPerson::debugInMenu()
{
    ImGui::DragFloat3("Offsets", &_clipping_offset.x, 0.1f, -20.f, 20.f);
    ImGui::DragFloat2("Angles", &_clamp_angle.x, 0.1f, -90.f, 90.f);
    ImGui::DragFloat("Speed", &_speed, 0.1f, 0.f, 20.f);
    ImGui::Text("Current euler %f - %f", rad2deg(_current_euler.x), rad2deg(_current_euler.y));

	ImGui::DragFloat("Time To Stop", &time_to_stop_shake, 0.1f, 0.f, 5.f);
	ImGui::DragFloat("Amount", &amount_shak, 0.01f, 0.f, 5.0f);
	ImGui::DragFloat("Speed_Shake", &speed_shak, 1.5f, 0.f, 200.f);
	if(ImGui::SmallButton("Shake")) {
		VHandles v_tp_cameras = CTagsManager::get().getAllEntitiesByTag(getID("tp_camera"));
		TMsgCameraShake msg;
		msg.amount = amount_shak;
		msg.speed = speed_shak;
		msg.time_to_stop = time_to_stop_shake;
		for (int i = 0; i < v_tp_cameras.size(); i++) {
			v_tp_cameras[i].sendMsg(msg);
		}
	}

}

void TCompCameraThirdPerson::load(const json& j, TEntityParseContext& ctx)
{
    // Read from the json all the input data
    _speed = j.value("speed", 1.5f);
    _target_name = j.value("target", "");
    _clamp_angle = loadVEC2(j["clampangle"]);
    _clipping_offset = loadVEC3(j["offset"]);
    _clamp_angle = VEC2(deg2rad(_clamp_angle.x), deg2rad(_clamp_angle.y));

    _h_target = getEntityByName(_target_name);
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
    DECL_MSG(TCompCameraThirdPerson, TMsgEntityCreated, onMsgCameraCreated);
    DECL_MSG(TCompCameraThirdPerson, TMsgEntitiesGroupCreated, onMsgCameraGroupCreated);
    DECL_MSG(TCompCameraThirdPerson, TMsgCameraResetTargetPos, onMsgCameraResetTargetPos);
    DECL_MSG(TCompCameraThirdPerson, TMsgCameraFov, onMsgCameraFov);
	DECL_MSG(TCompCameraThirdPerson, TMsgCameraShake, onMsgCameraShaked);
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

void TCompCameraThirdPerson::onMsgCameraGroupCreated(const TMsgEntitiesGroupCreated & msg) {

    // Load the target and set his axis as our axis.
    //_h_target = getEntityByName(_target_name);
    //TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>();

    //float yaw, pitch, roll;
    //target_transform->getYawPitchRoll(&yaw, &pitch, &roll);
    //_current_euler = VEC2(yaw, pitch);
    //_original_euler = _current_euler;
}

void TCompCameraThirdPerson::onMsgCameraShaked(const TMsgCameraShake &msg) {
	activateCameraShake(msg.amount, msg.speed, msg.time_to_stop);
}

void TCompCameraThirdPerson::onMsgCameraCreated(const TMsgEntityCreated & msg) {

    // Load the target and set his axis as our axis.
    //_h_target = getEntityByName(_target_name);
    //TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>();

    //float yaw, pitch, roll;
    //target_transform->getYawPitchRoll(&yaw, &pitch, &roll);
    //_current_euler = VEC2(yaw, pitch);
    //_original_euler = _current_euler;
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
    resetCamera(msg.both_angles, msg.only_y);
}

void TCompCameraThirdPerson::onMsgCameraResetTargetPos(const TMsgCameraResetTargetPos & msg)
{
    resetCameraTargetPos();
}

void TCompCameraThirdPerson::onMsgCameraFov(const TMsgCameraFov & msg)
{
    _target_fov = msg.new_fov;
    _max_time_fov = msg.blend_time;
    _timer_fov = 0.f;
}

float TCompCameraThirdPerson::getFovUpdated(float dt)
{
    _timer_fov = Clamp(_timer_fov + dt, 0.f, _max_time_fov);
    float new_fov = lerp(getFov(), deg2rad(_target_fov), _timer_fov / _max_time_fov);
    //dbg("===================================================================================\n");
    //dbg("UPDATING FOV: %f\n", rad2deg(new_fov));
    //
    //float inputSpeed = Clamp(fabs(btHorizontal.value) + fabs(btVertical.value), 0.f, 1.f);
    //float current_fov = 70 + inputSpeed * 30; // Just doing some testing with the fov and speed
    //                                          //setPerspective(getFovUpdated(dt), 0.1f, 1000.f);
    //                                          //dbg("Fov %f %f\n", current_fov, inputSpeed);

    return new_fov;
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
        CEntity* e_target = _h_target;
        TCompAIPlayer* ai_player = e_target->get<TCompAIPlayer>();
        if (!(ai_player && ai_player->enabledPlayerAI)) {
            _current_euler.x -= horizontal_delta * _speed * dt;
            _current_euler.y += vertical_delta * _speed * dt;
            _current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, -_clamp_angle.x);
        }

        // EulerAngles method based on mcv class
        VEC3 vertical_offset = VEC3::Up * _clipping_offset.y; // Change VEC3::up, for the players vertical angle, (TARGET VERTICAL)
        VEC3 horizontal_offset = self_transform->getLeft() * _clipping_offset.x;
        VEC3 target_position = target_transform->getPosition() + vertical_offset + horizontal_offset;
        self_transform->setYawPitchRoll(_current_euler.x, _current_euler.y, 0);

        float z_distance = CameraClipping(target_position, -self_transform->getFront());
        VEC3 new_pos = target_position + z_distance * -self_transform->getFront();
        self_transform->setPosition(new_pos);

		CEntity* e = CHandle(this).getOwner();
		//dbg("%s before activate\n", e->getName());
		
        //float inputSpeed = Clamp(fabs(btHorizontal.value) + fabs(btVertical.value), 0.f, 1.f);
        //float current_fov = 70 + inputSpeed * 30; // Just doing some testing with the fov and speed
        setPerspective(getFovUpdated(dt), 0.1f, 1000.f);
        //dbg("Setting perspective TP() - new fov: %f\n", rad2deg(getFov()));
    }

    if (!paused || activate_shake && CEngine::get().getGameManager().getCurrentState() == CModuleGameManager::PauseState::defeat) {
        if (activate_shake) {

            //dbg("%s on activate\n", e->getName());
            TCompTransform* self_transform = get<TCompTransform>();
            _time_shaking += dt;
            shake_percentage = (time_to_stop_shake - _time_shaking) / time_to_stop_shake;
            float x_amount = sin(_time_shaking * speed_shak) * amount_shak * shake_percentage;
            VEC3 shaking_pos = self_transform->getPosition();
            shaking_pos += self_transform->getUp() * x_amount;
            self_transform->setPosition(shaking_pos);
            if ((time_to_stop_shake - _time_shaking) <= 0.0f) {
                activate_shake = false;
                _time_shaking = 0.0f;
            }

        }
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

void TCompCameraThirdPerson::resetCamera(bool both_angles, bool only_y)
{
    if (both_angles) {
        //_current_euler = _original_euler;
        _current_euler.x = _original_euler.x;
        _current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, _original_euler.y);
    }
    else if (only_y) {
        //_current_euler.y = _original_euler.y;
        _current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, _original_euler.y);
    }
    else {
        _current_euler.x = _original_euler.x;
    }
}

void TCompCameraThirdPerson::resetCameraTargetPos()
{
    _h_target = getEntityByName(_target_name);
    TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>();

    float yaw, pitch;
    target_transform->getYawPitchRoll(&yaw, &pitch);
    _current_euler.x = yaw;
    _current_euler.y = _original_euler.y;
}

void TCompCameraThirdPerson::activateCameraShake(float amount_shake, float speed_shake, float time_to_stop) {
    if (amount_shak * shake_percentage < amount_shake) {
	    activate_shake = true;
	    _time_shaking = 0.0f;
        amount_shak = amount_shake;
        speed_shak = speed_shake;
        time_to_stop_shake = time_to_stop;
    }
}

void TCompCameraThirdPerson::onPause(const TMsgScenePaused& msg) {

    paused = msg.isPaused;
}


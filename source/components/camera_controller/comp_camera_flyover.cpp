#include "mcv_platform.h"
#include "comp_camera_flyover.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("camera_flyover", TCompCameraFlyover);

void TCompCameraFlyover::debugInMenu() {
    ImGui::DragFloat("Speed", &_speed, 10.f, 1.f, 100.f);
    ImGui::DragFloat("Sensitivity", &_sensitivity, 0.01f, 0.001f, 0.1f);
}

void TCompCameraFlyover::load(const json& j, TEntityParseContext& ctx) {
    _speed = j.value("speed", _speed);
    _sensitivity = j.value("sensitivity", _sensitivity);
}

void TCompCameraFlyover::update(float dt)
{
    TCompTransform* c_transform = get<TCompTransform>();
    if (!c_transform)
        return;

    /*VEC3 pos = c_transform->getPosition();
    VEC3 front = c_transform->getFront();
    VEC3 left = c_transform->getLeft();
    VEC3 up = VEC3::UnitY;*/

    if (btDebugPause.getsPressed()) {
        paused = !paused;
        if (paused) {
            TMsgScenePaused msg;
			msg.isPaused = true;

            //EngineEntities.broadcastMsg(msg);
            CEntity * player = getEntityByName("The Player");
            player->sendMsg(msg);
            Engine.getCameras().blendInCamera(CHandle(this).getOwner(), 1.f, CModuleCameras::EPriority::TEMPORARY);

        }
        else {
			Engine.getCameras().blendOutCamera(CHandle(this).getOwner(),1.0f);
            TMsgScenePaused msg;
            msg.isPaused = false;
            EngineEntities.broadcastMsg(msg);
            CEntity * player = CTagsManager::get().getAllEntitiesByTag(getID("player"))[0];

           // Engine.getCameras().blendOutCamera(CHandle(this).getOwner(), 1.f);
        }
    }

    if (paused) {

        // movement
        float deltaSpeed = _speed * dt;
        if (EngineInput["btRun"].isPressed())
            deltaSpeed *= 3.f;

		VEC3 pos = c_transform->getPosition();
		VEC3 front = c_transform->getFront();
		VEC3 left = c_transform->getLeft();
		VEC3 up = VEC3::UnitY;
        VEC3 off;
        off += front * EngineInput["btUp"].value * deltaSpeed;
        off += -front * EngineInput["btDown"].value * deltaSpeed;
        off += left * EngineInput["btLeft"].value * deltaSpeed;
        off += -left * EngineInput["btRight"].value * deltaSpeed;
        off += up * EngineInput["up"].value * deltaSpeed;
        off += -up * EngineInput["down"].value * deltaSpeed;

        // rotation
        float yaw, pitch;
        getYawPitchFromVector(front, &yaw, &pitch);

        auto& mouse = EngineInput.mouse();
        if (mouse.button(Input::MOUSE_RIGHT).isPressed())
        {
            VEC2 mOff = mouse._position_delta;
            yaw += -mOff.x * _sensitivity;
            pitch += -mOff.y * _sensitivity;
            pitch = Clamp(pitch, -_maxPitch, _maxPitch);
        }

        // final values
        VEC3 newPos = pos + off;
        VEC3 newFront = getVectorFromYawPitch(yaw, pitch);

        c_transform->lookAt(newPos, newPos + newFront);

        if (btDebugGetPos.getsPressed()) {
            dbg("Camera pos - \"%f %f %f\"\n", newPos.x, newPos.y, newPos.z);
        }
    }

}

void TCompCameraFlyover::registerMsgs() {
	DECL_MSG(TCompCameraFlyover, TMsgCameraActivated, onMsgActivatedMyself);
}

void TCompCameraFlyover::onMsgActivatedMyself(const TMsgCameraActivated & msg) {

	CEntity * player = getEntityByName("The Player");
	TCompTransform* c_player_transform = player->get<TCompTransform>();
	TCompTransform* c_transform = get<TCompTransform>();
	c_transform->setPosition(c_player_transform->getPosition() + Vector3::Up * 2.0f);
	c_transform->setRotation(c_player_transform->getRotation());
}
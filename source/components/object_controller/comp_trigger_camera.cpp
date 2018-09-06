#include "mcv_platform.h"
#include "comp_trigger_camera.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("trigger_camera", TCompTriggerCamera);

void TCompTriggerCamera::debugInMenu() {
}

void TCompTriggerCamera::load(const json& j, TEntityParseContext& ctx) {

	_targetName = j.value("target", "");
	_timeToExitCamera = j.value("time_to_exit", 0.f);
	_targetCamera = getEntityByName(_targetName);
	_blendInTime = j.value("blend_in_time", 0.f);
	_blendOutTime = j.value("blend_out_time", 0.f);
}

void TCompTriggerCamera::registerMsgs()
{
	DECL_MSG(TCompTriggerCamera, TMsgTriggerEnter, onMsgTriggerEnter);
	DECL_MSG(TCompTriggerCamera, TMsgTriggerExit, onMsgTriggerExit);
}

void TCompTriggerCamera::onMsgTriggerEnter(const TMsgTriggerEnter & msg)
{
	if (!used) {
		Engine.getCameras().blendInCamera(_targetCamera, _blendInTime, CModuleCameras::EPriority::CINEMATIC);
		TMsgScenePaused stopPlayer;
		stopPlayer.isPaused = true;
		EngineEntities.broadcastMsg(stopPlayer);
		onCamera = true;
		used = true;
	}
}

void TCompTriggerCamera::onMsgTriggerExit(const TMsgTriggerExit & msg)
{
	//Engine.getCameras().blendOutCamera(_targetCamera, _blendOutTime);
}

void TCompTriggerCamera::update(float dt)
{
	if (onCamera) {
		time += dt;
		if (time >= _timeToExitCamera) {
			if (firstFrame) {
				Engine.getCameras().blendOutCamera(_targetCamera, _blendOutTime);
				firstFrame = false;
			}
			if (time >= _timeToExitCamera + _blendOutTime) {
				onCamera = false;
				TMsgScenePaused stopPlayer;
				stopPlayer.isPaused = false;
				EngineEntities.broadcastMsg(stopPlayer);
				time = 0.0f;
			}		
		}
	}
}

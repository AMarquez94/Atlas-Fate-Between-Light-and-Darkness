#include "mcv_platform.h"
#include "comp_trigger_camera.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("trigger_camera", TCompTriggerCamera);

void TCompTriggerCamera::debugInMenu() {
}

void TCompTriggerCamera::load(const json& j, TEntityParseContext& ctx) {

	_targetName = j.value("target", "");
	_targetCamera = getEntityByName(_targetName);
}

void TCompTriggerCamera::registerMsgs()
{
	DECL_MSG(TCompTriggerCamera, TMsgTriggerEnter, onMsgTriggerEnter);
	DECL_MSG(TCompTriggerCamera, TMsgTriggerExit, onMsgTriggerExit);
}

void TCompTriggerCamera::onMsgTriggerEnter(const TMsgTriggerEnter & msg)
{
	Engine.getCameras().blendInCamera(_targetCamera, 0.f, CModuleCameras::EPriority::TEMPORARY);
}

void TCompTriggerCamera::onMsgTriggerExit(const TMsgTriggerExit & msg)
{
	Engine.getCameras().blendOutCamera(_targetCamera, 1.f);
}

void TCompTriggerCamera::update(float dt)
{
}

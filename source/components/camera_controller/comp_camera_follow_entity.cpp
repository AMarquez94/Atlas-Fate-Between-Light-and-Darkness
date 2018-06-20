#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_camera_follow_entity.h"
#include "components/comp_transform.h"

DECL_OBJ_MANAGER("camera_follow", TCompCameraFollowEntity);

void TCompCameraFollowEntity::debugInMenu()
{
}

void TCompCameraFollowEntity::load(const json& j, TEntityParseContext& ctx)
{
	// Read from the json all the input data
	//_speed = j.value("speed", 1.5f);
	_target_name = j.value("target", "");
	//_clamp_angle = loadVEC2(j["clampangle"]);
	//_clipping_offset = loadVEC3(j["offset"]);
	//_clamp_angle = VEC2(deg2rad(_clamp_angle.x), deg2rad(_clamp_angle.y));

	// Load the target and set his axis as our axis.
	_h_target = ctx.findEntityByName(_target_name);
    offset = loadVEC3(j["offset"]);

	active = false;
}

void TCompCameraFollowEntity::registerMsgs()
{
	DECL_MSG(TCompCameraFollowEntity, TMsgCameraActivated, onMsgCameraActive);
	DECL_MSG(TCompCameraFollowEntity, TMsgCameraDeprecated, onMsgCameraDeprecated);
	DECL_MSG(TCompCameraFollowEntity, TMsgCameraFullyActivated, onMsgCameraFullActive);
	DECL_MSG(TCompCameraFollowEntity, TMsgSetCameraActive, onMsgCameraSetActive);
	DECL_MSG(TCompCameraFollowEntity, TMsgScenePaused, onPause);
}

void TCompCameraFollowEntity::onMsgCameraActive(const TMsgCameraActivated & msg)
{
}

void TCompCameraFollowEntity::onMsgCameraFullActive(const TMsgCameraFullyActivated & msg)
{
	active = true;
}

void TCompCameraFollowEntity::onMsgCameraDeprecated(const TMsgCameraDeprecated & msg)
{
	active = false;
}

void TCompCameraFollowEntity::onMsgCameraSetActive(const TMsgSetCameraActive & msg)
{
}

void TCompCameraFollowEntity::update(float dt)
{
	if (!paused) {

		if (!_h_target.isValid()) return;

        TCompTransform* tpos = get<TCompTransform>();
        CEntity* eTarget = _h_target;
        TCompTransform* targetPos = eTarget->get<TCompTransform>();
        tpos->setPosition(targetPos->getPosition() - targetPos->getFront() * 3 + VEC3(0,offset.y,0));

        tpos->lookAt(tpos->getPosition(), targetPos->getPosition());
	}
}

void TCompCameraFollowEntity::onPause(const TMsgScenePaused& msg) {

	paused = msg.isPaused;
}


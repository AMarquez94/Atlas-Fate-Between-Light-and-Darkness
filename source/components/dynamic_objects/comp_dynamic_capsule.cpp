#include "mcv_platform.h"
#include "comp_dynamic_capsule.h"
#include "components\comp_transform.h"

DECL_OBJ_MANAGER("dynamic_capsule", TCompDynamicCapsule);

void TCompDynamicCapsule::onMsgScenePaused(const TMsgScenePaused & msg)
{
    paused = msg.isPaused;
}

void TCompDynamicCapsule::debugInMenu() {

}

void TCompDynamicCapsule::load(const json& j, TEntityParseContext& ctx) {

	speed = j.value("speed", 0.f);
	start_point = loadVEC3(j["start_point"]);
	end_point = loadVEC3(j["end_point"]);

    director = end_point - start_point;
    director.Normalize();
}

void TCompDynamicCapsule::update(float dt) {

    if (!paused) {
        VEC3 dir = end_point - start_point;
        dir.Normalize();

        TCompTransform *myPos = get<TCompTransform>();
        myPos->setPosition(myPos->getPosition() + dir * speed * dt);

        if (VEC3::Distance(myPos->getPosition(), end_point) < speed * dt) {
            myPos->setPosition(start_point);
        }
        //if (dir.Dot(facing_dir) < 0)
        //    myPos->setPosition(start_point);
    }
}

void TCompDynamicCapsule::setSpeed(float newSpeed) {

	speed = newSpeed;
}

void TCompDynamicCapsule::setStartPoint(VEC3 newStartPoint) {

	start_point = newStartPoint;
}

void TCompDynamicCapsule::setEndPoint(VEC3 newEndPoint) {

	end_point = newEndPoint;
}

void TCompDynamicCapsule::setOffset(VEC3 newOffset)
{
  offset = newOffset;
}

void TCompDynamicCapsule::registerMsgs()
{
    DECL_MSG(TCompDynamicCapsule, TMsgScenePaused, onMsgScenePaused);
}

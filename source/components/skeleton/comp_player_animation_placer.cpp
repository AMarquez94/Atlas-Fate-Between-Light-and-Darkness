#include "mcv_platform.h"
#include "components/skeleton/comp_player_animation_placer.h"


DECL_OBJ_MANAGER("player_animator_placer", TCompPlayerAnimatorPlacer);

void  TCompPlayerAnimatorPlacer::debugInMenu() {

}

void TCompPlayerAnimatorPlacer::load(const json& j, TEntityParseContext& ctx) {
	pointPosition = loadVEC3(j.value("relative_position", "0 0 0"));
	pointRotation; 

}

void TCompPlayerAnimatorPlacer::update(float dt) {


}

void TCompPlayerAnimatorPlacer::registerMsgs() {

}

VEC3 TCompPlayerAnimatorPlacer::getPointPosition() {

	return pointPosition;
}

QUAT TCompPlayerAnimatorPlacer::getPointRotation() {

	return pointRotation;
}

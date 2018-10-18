#include "mcv_platform.h"
#include "components/skeleton/comp_player_animation_placer.h"


DECL_OBJ_MANAGER("player_animator_placer", TCompPlayerAnimatorPlacer);

void  TCompPlayerAnimatorPlacer::debugInMenu() {

}

void TCompPlayerAnimatorPlacer::load(const json& j, TEntityParseContext& ctx) {
	pointPosition = loadVEC3(j["relative_position"]);

}

void TCompPlayerAnimatorPlacer::update(float dt) {


}

void TCompPlayerAnimatorPlacer::registerMsgs() {
	DECL_MSG(TCompPlayerAnimatorPlacer, TMsgEntitiesGroupCreated, onMsgGroupCreated);
}

VEC3 TCompPlayerAnimatorPlacer::getPointPosition() {

	return pointPosition;
}

VEC3 TCompPlayerAnimatorPlacer::getPointToLookAt() {

	return pointToLookAt;
}

void TCompPlayerAnimatorPlacer::setPointToLookAt(VEC3 point) {
	pointToLookAt = point;
}

void TCompPlayerAnimatorPlacer::onMsgGroupCreated(const TMsgEntitiesGroupCreated& msg) {
	CEntity *e = CHandle(this).getOwner();
	TCompTransform *c_trans = e->get<TCompTransform>();
	pointToLookAt = c_trans->getPosition();

	VEC3 aux = VEC3::Zero;
	aux += c_trans->getFront() * pointPosition.z;
	aux += c_trans->getLeft() * pointPosition.x;
	pointPosition = c_trans->getPosition() + aux;
	
}

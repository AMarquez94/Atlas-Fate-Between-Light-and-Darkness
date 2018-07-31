#include "mcv_platform.h"
#include "components/object_controller/comp_laser.h"

DECL_OBJ_MANAGER("laser", TCompLaser);

void  TCompLaser::debugInMenu() {

}

void TCompLaser::load(const json& j, TEntityParseContext& ctx) {

}

void TCompLaser::update(float dt) {
	CEntity *e = CHandle(this).getOwner();
	TCompTransform * comp_transform = e->get<TCompTransform>();
	dbg("ei");
}

void TCompLaser::registerMsgs() {

}
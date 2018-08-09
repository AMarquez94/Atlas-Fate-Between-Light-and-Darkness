#include "mcv_platform.h"
#include "components/object_controller/comp_laser.h"
#include "components/comp_transform.h"

DECL_OBJ_MANAGER("laser", TCompLaser);

void  TCompLaser::debugInMenu() {

}

void TCompLaser::load(const json& j, TEntityParseContext& ctx) {

}

void TCompLaser::update(float dt) {
	CEntity* e = CHandle(this).getOwner();
	TCompTransform *mypos = e->get<TCompTransform>();
	physx::PxRaycastHit hit;
	VEC3 origin = mypos->getPosition();
	VEC3 dir = mypos->getFront();
	if (EnginePhysics.Raycast(origin, dir, 40.0f, hit, physx::PxQueryFlag::eSTATIC)) {
		mypos->setScale( VEC3( 1, 1, (float)hit.distance ) );
	}
		
}

void TCompLaser::registerMsgs() {

}
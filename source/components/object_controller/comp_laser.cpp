#include "mcv_platform.h"
#include "components/object_controller/comp_laser.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/ia/comp_bt_mimetic.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("laser", TCompLaser);

void  TCompLaser::debugInMenu() {

}

void TCompLaser::load(const json& j, TEntityParseContext& ctx) {

	
	parent_name = j.value("parent","Mimetic");

	physx::PxFilterData pxFilterData;
	pxFilterData.word0 = FilterGroup::All;
	laserFilter.data = pxFilterData;

	h_parent = ctx.findEntityByName(parent_name);
}

void TCompLaser::update(float dt) {

	playerDetected = false;

	CEntity* e = CHandle(this).getOwner();
	if (e == nullptr) return;
	TCompTransform *mypos = e->get<TCompTransform>();
	physx::PxRaycastHit hit;
	VEC3 origin = mypos->getPosition();
	VEC3 dir = -mypos->getUp();
	if (EnginePhysics.Raycast(origin, dir, 40.0f, hit, (physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC), laserFilter)) {
		mypos->setScale( VEC3( 1, (float)hit.distance, 1 ) );
		CHandle possible_player;
		possible_player.fromVoidPtr(hit.actor->userData);
		possible_player = possible_player.getOwner();
		CEntity *e_player = possible_player;
		if (e_player == nullptr) return;
		TCompTempPlayerController * c_player = e_player->get<TCompTempPlayerController>();
		if (c_player != nullptr) {
			playerDetected = true;
		}
		
	}
	else {
		mypos->setScale(VEC3(1, 40.0, 0));
	}
		
}

void TCompLaser::registerMsgs() {
	DECL_MSG(TCompLaser, TMsgEntitiesGroupCreated, onEntityCreated);
}

bool TCompLaser::isPlayerOnLaser() {
	return playerDetected;
}

void TCompLaser::onEntityCreated(const TMsgEntitiesGroupCreated& msg) {	
	
	CEntity *e = h_parent;
	TCompAIMimetic *comp_ia = e->get<TCompAIMimetic>();
	if (comp_ia != nullptr) {
		CEntity *en = CHandle(this).getOwner();
		comp_ia->registerLaserHandle(CHandle(this).getOwner());
	}

}
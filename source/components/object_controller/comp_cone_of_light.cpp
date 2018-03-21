#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_cone_of_light.h"
#include "components/comp_transform.h"
#include "components/comp_tags.h"
#include "components/comp_render.h"
#include "components/player_controller/comp_player_controller.h"
#include "render/mesh/mesh_loader.h"
#include "render/render_utils.h"

DECL_OBJ_MANAGER("cone_of_light", TCompConeOfLightController);

void TCompConeOfLightController::debugInMenu() {

	if (origin != VEC3::Zero && dest != VEC3::Zero) {
		renderLine(origin, dest, VEC4(0, 1, 0, 1));
	}
}

void TCompConeOfLightController::load(const json& j, TEntityParseContext& ctx) {
	fov = deg2rad(j.value("fov", 30.f));
	dist = j.value("dist", 10.f);
	player = (CEntity*) getEntityByName(j.value("target", "The Player"));
	turnedOn = j.value("turnedOn", false);
}

void TCompConeOfLightController::registerMsgs() {
	DECL_MSG(TCompConeOfLightController, TMsgEntityCreated, onMsgEntityCreated);
}

void TCompConeOfLightController::onMsgEntityCreated(const TMsgEntityCreated& msg) {
	if (!turnedOn) {
		TCompRender* cRender = get < TCompRender>();
		cRender->visible = false;
	}
}

void TCompConeOfLightController::update(float dt) {
	//if (turnedOn) {
	//	TCompPlayerController* pController = player->get<TCompPlayerController>();
	//	if (pController->isInShadows()) {
	//		TCompTransform* ppos = player->get<TCompTransform>();
	//		TCompTransform* mypos = get<TCompTransform>();
	//		bool inDist = VEC3::Distance(mypos->getPosition(), ppos->getPosition()) < dist;
	//		if (VEC3::Distance(mypos->getPosition(), ppos->getPosition()) < dist 
	//			&& mypos->isInFov(ppos->getPosition(), fov)) {
	//			if (!isPlayerHiddenFromLight(player)) {
	//				TMsgPlayerIlluminated msg;
	//				msg.h_sender = CHandle(this).getOwner();
	//				player->sendMsg(msg);
	//			}
	//			else {
	//				dbg("That bastard is hidden\n");
	//			}
	//		}
	//	}
	//}
}

void TCompConeOfLightController::turnOnLight() {
	if (!turnedOn) {
		TCompRender* cRender = get < TCompRender>();
		cRender->visible = true;
		turnedOn = true;
	}
}

void TCompConeOfLightController::turnOffLight() {
	if (turnedOn) {
		TCompRender* cRender = get < TCompRender>();
		cRender->visible = false;
		turnedOn = false;
	}
}

bool TCompConeOfLightController::isPlayerHiddenFromLight(CEntity* player)
{
	CEntity* parent = CHandle(this).getOwner().getOwner();
	TCompTransform *mypos = parent->get<TCompTransform>();
	TCompTransform *pTransform = player->get<TCompTransform>();
	TCompCollider *myCollider = parent->get<TCompCollider>();
	TCompCollider *pCollider = player->get<TCompCollider>();

	VEC3 myPosition = mypos->getPosition();

	origin = myPosition + VEC3(0, 3 + .1f, 0);
	dest = pTransform->getPosition() + VEC3(0,.1f,0);
	VEC3 dir = dest - origin;
	dir.Normalize();
	float distance = VEC3::Distance(origin, dest);
	
	physx::PxRaycastHit hit;

	//TODO: only works when behind scenery. Make the same for other enemies, dynamic objects...
	return EnginePhysics.Raycast(origin, dir, distance, hit, physx::PxQueryFlag::eSTATIC);
}
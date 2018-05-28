#include "mcv_platform.h"
#include "comp_trigger_checkpoint.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"
#include "modules/game/module_game_manager.h"

DECL_OBJ_MANAGER("trigger_checkpoint", TCompTriggerCheckpoint);

void TCompTriggerCheckpoint::debugInMenu() {
}

void TCompTriggerCheckpoint::load(const json& j, TEntityParseContext& ctx) {
}

void TCompTriggerCheckpoint::registerMsgs()
{
	DECL_MSG(TCompTriggerCheckpoint, TMsgTriggerEnter, onMsgTriggerEnter);
}

void TCompTriggerCheckpoint::onMsgTriggerEnter(const TMsgTriggerEnter & msg)
{
	if (!used) {
		CHandle h_player = getEntityByName("The Player");
		if (h_player == msg.h_other_entity) {
			CModuleGameManager gameManager = CEngine::get().getGameManager();
			CEntity* e_player = h_player;
			TCompTransform* tPlayerPos = e_player->get<TCompTransform>();
			TCompTransform* myPos = get<TCompTransform>();
			/* TODO: Que sea con la rotacion del player o la que especifiquemos nosotros */
			gameManager.saveCheckpoint(VEC3(myPos->getPosition().x, tPlayerPos->getPosition().y, myPos->getPosition().z), tPlayerPos->getRotation());
			used = true;
		}
	}
}

void TCompTriggerCheckpoint::update(float dt)
{

}

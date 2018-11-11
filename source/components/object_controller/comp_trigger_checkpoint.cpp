#include "mcv_platform.h"
#include "comp_trigger_checkpoint.h"
#include "components/comp_transform.h"
#include "entity/common_msgs.h"
#include "modules/game/module_game_manager.h"

DECL_OBJ_MANAGER("trigger_checkpoint", TCompTriggerCheckpoint);

void TCompTriggerCheckpoint::debugInMenu() {
}

void TCompTriggerCheckpoint::load(const json& j, TEntityParseContext& ctx) {
    player_pos = loadVEC3(j.value("player_pos", "0 0 0"));
    player_lookat = loadVEC3(j.value("player_lookat", "0 0 0"));
}

void TCompTriggerCheckpoint::registerMsgs()
{
	DECL_MSG(TCompTriggerCheckpoint, TMsgTriggerEnter, onMsgTriggerEnter);
}

void TCompTriggerCheckpoint::onMsgTriggerEnter(const TMsgTriggerEnter & msg)
{
	if (CHandle(this).getOwner().isValid() && !used) {
		CHandle h_player = EngineEntities.getPlayerHandle();
		if (h_player == msg.h_other_entity) {
			CModuleGameManager gameManager = CEngine::get().getGameManager();
			CEntity* e_player = h_player;
			TCompTransform* tPlayerPos = e_player->get<TCompTransform>();
			TCompTransform* myPos = get<TCompTransform>();
			/* TODO: Que sea con la rotacion del player o la que especifiquemos nosotros */
            CEntity* my_entity = CHandle(this).getOwner();
            VEC3 checkpoint_player_pos = player_pos == VEC3::Zero ? VEC3(myPos->getPosition().x, tPlayerPos->getPosition().y, myPos->getPosition().z) : VEC3(player_pos.x, tPlayerPos->getPosition().y, player_pos.z);
            VEC3 checkpoint_player_lookat = player_lookat == VEC3::Zero ? checkpoint_player_pos + tPlayerPos->getFront() : VEC3(player_lookat.x, checkpoint_player_pos.y, player_lookat.z);
            gameManager.saveCheckpoint(my_entity->getName(), checkpoint_player_pos, checkpoint_player_lookat);
			used = true;
		}
	}
}

void TCompTriggerCheckpoint::update(float dt)
{

}

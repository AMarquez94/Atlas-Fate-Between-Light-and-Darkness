#include "mcv_platform.h"
#include "comp_player_input.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("player_input", TCompPlayerInput);

void TCompPlayerInput::debugInMenu() {
}

void TCompPlayerInput::load(const json& j, TEntityParseContext& ctx) {
}

void TCompPlayerInput::update(float dt)
{
	CEntity* e = CHandle(this).getOwner();

	/* Movement messages*/
	{
		TMsgSetFSMVariable walkMsg;
		walkMsg.variant.setName("speed");

		if (EngineInput["btUp"].hasChanged()){
			walkMsg.variant.setFloat(EngineInput["btUp"].value);
			e->sendMsg(walkMsg);
		}
		else if (EngineInput["btDown"].hasChanged()) {
			walkMsg.variant.setFloat(EngineInput["btDown"].value);
			e->sendMsg(walkMsg);
		}
		else if (EngineInput["btLeft"].hasChanged()) {
			walkMsg.variant.setFloat(EngineInput["btLeft"].value);
			e->sendMsg(walkMsg);
		}
		else if (EngineInput["btRight"].hasChanged()) {
			walkMsg.variant.setFloat(EngineInput["btRight"].value);
			e->sendMsg(walkMsg);
		}

		/* Speed boost messages */
		TMsgSetFSMVariable boostMsg;
		boostMsg.variant.setName("boost_speed");

		if (EngineInput["btRun"].hasChanged()) {
			boostMsg.variant.setFloat(EngineInput["btRun"].value);
			e->sendMsg(boostMsg);
		}

		if (EngineInput["btSlow"].hasChanged()) {
			boostMsg.variant.setFloat(EngineInput["btSlow"].value);
			e->sendMsg(boostMsg);
		}
	}

	/* Shadow merge messages*/
	{
		TMsgSetFSMVariable shadowMerge;
		shadowMerge.variant.setName("merge");
		shadowMerge.variant.setBool(EngineInput["btShadowMerging"].isPressed());

		if (EngineInput["btShadowMerging"].hasChanged())
		{
			e->sendMsg(shadowMerge);
		}
	}

	/* Player/User interaction messages */
	{

		if (EngineInput["btAttack"].hasChanged())
		{
			TMsgSetFSMVariable attack;
			attack.variant.setName("attack");
			attack.variant.setBool(EngineInput["btAttack"].isPressed());
			e->sendMsg(attack);
		}

		if (EngineInput["btCrouch"].hasChanged())
		{
			TMsgSetFSMVariable crouch;
			crouch.variant.setName("crouch");
			crouch.variant.setBool(EngineInput["btCrouch"].value);
			e->sendMsg(crouch);
		}

		if (EngineInput["btAction"].hasChanged())
		{
			TMsgSetFSMVariable action;
			action.variant.setName("action");
			action.variant.setBool(EngineInput["action"].isPressed());
			e->sendMsg(action);
		}

		if (EngineInput["btSecAction"].hasChanged())
		{
			TMsgSetFSMVariable action;
			action.variant.setName("action_sec");
			action.variant.setBool(EngineInput["btSecAction"].isPressed());
			e->sendMsg(action);
		}
	}

}

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
		if (EngineInput["btUp"].hasChanged() 
			|| EngineInput["btDown"].hasChanged() 
			|| EngineInput["btLeft"].hasChanged() 
			|| EngineInput["btRight"].hasChanged()){

			TMsgSetFSMVariable walkMsg;
			walkMsg.variant.setName("speed");
			float total_value = (EngineInput["btRight"].value + EngineInput["btLeft"].value
								+ EngineInput["btUp"].value + EngineInput["btDown"].value);

			walkMsg.variant.setFloat(total_value);
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
		if (EngineInput["btShadowMerging"].hasChanged())
		{
			TMsgSetFSMVariable shadowMerge;
			shadowMerge.variant.setName("onmerge");
			shadowMerge.variant.setBool(EngineInput["btShadowMerging"].isPressed());
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

		if (EngineInput["btCrouch"].getsPressed())
		{
			crouchButton = !crouchButton;
			TMsgSetFSMVariable crouch;
			crouch.variant.setName("crouch");
			crouch.variant.setBool(crouchButton);
			e->sendMsg(crouch);
		}

		if (EngineInput["btAction"].hasChanged())
		{
			TMsgSetFSMVariable action;
			action.variant.setName("action");
			action.variant.setBool(true);
			e->sendMsg(action);
		}

		if (EngineInput["btSecAction"].getsPressed())
		{
			inhibition++;
			TMsgSetFSMVariable action;
			action.variant.setName("action_sec");
			action.variant.setFloat(inhibition);
			e->sendMsg(action);
			if (inhibition == 5) inhibition = 0;
		}
	}

}

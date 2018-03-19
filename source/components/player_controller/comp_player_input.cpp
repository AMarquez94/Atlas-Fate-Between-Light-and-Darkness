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
	if (EngineInput["btUp"].hasChanged())
	{
		TMsgSetFSMVariable jumpMsg;
		jumpMsg.variant.setName("speed");
		jumpMsg.variant.setFloat(EngineInput["btUp"].value);
		CEntity* e = CHandle(this).getOwner();
		e->sendMsg(jumpMsg);
	}

	if (EngineInput["btDown"].hasChanged())
	{
		TMsgSetFSMVariable jumpMsg;
		jumpMsg.variant.setName("speedx");
		jumpMsg.variant.setFloat(-EngineInput["btDown"].value);
		CEntity* e = CHandle(this).getOwner();
		e->sendMsg(jumpMsg);
	}

	if (EngineInput["jump"].hasChanged())
	{
		//TMsgSetFSMVariable speedMsg;
		//speedMsg.variant.setName("speed");
		//speedMsg.variant.setFloat(EngineInput["move"].value);
		//CEntity* e = CHandle(this).getOwner();
		//e->sendMsg(speedMsg);
	}
}

#pragma once

#include "comp_base.h"
#include "fsm/context.h"


struct TMsgSetFSMVariable {
	CVariant variant;
	DECL_MSG_ID();
};

class TCompFSM : public TCompBase {
public:

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	void onPaused(const TMsgScenePaused& msg);
	void onCreate(const TMsgEntityCreated& msg);
	void onVariableValue(const TMsgSetFSMVariable& msg);

	static void registerMsgs();

	const std::string getStateName();

private:

	FSM::CContext _context;
	DECL_SIBLING_ACCESS();
};
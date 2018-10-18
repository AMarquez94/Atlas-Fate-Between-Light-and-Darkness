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
	void onConsoleChanged(const TMsgConsoleOn& msg);
    void onMsgNoClipToggle(const TMsgNoClipToggle& msg);
    void onMsgAIMode(const TMsgPlayerAIEnabled& msg);

	static void registerMsgs();

	const std::string getStateName();

private:

    bool isInNoClipMode = false;
    bool isInAIMode = false;
	FSM::CContext _context;

    /* TODO: Temp - Borrar. Solo con motivo de debug */
    std::vector<HistoricalAction> historic;
    void addActionToHistoric(const std::string & action);

	DECL_SIBLING_ACCESS();
};
#pragma once

#include "components/comp_base.h"
#include "comp_bt.h"

class TCompAITest : public TCompIAController {

private:

	DECL_SIBLING_ACCESS();

	void onMsgEntityCreated(const TMsgEntityCreated& msg);

public:
	void load(const json& j, TEntityParseContext& ctx) override;
	void debugInMenu();

	int actionIdle(float dt);
	int actionEscape(float dt);
	int actionShoot(float dt);
	int actionShootGrenade(float dt);
	int actionShootPistol(float dt);
	int actionPursuit(float dt);
	bool conditionEscape(float dt);


	void waitInput();

	static void registerMsgs();
};
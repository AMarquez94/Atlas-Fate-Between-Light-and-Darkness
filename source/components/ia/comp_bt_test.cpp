#include "mcv_platform.h"
#include "comp_bt_test.h"
#include "btnode.h"
#include "components/comp_name.h"
#include <conio.h>
#include <stdio.h>
#include <iostream>

DECL_OBJ_MANAGER("ai_test", TCompAITest);

void TCompAITest::debugInMenu() {

}


void TCompAITest::load(const json& j, TEntityParseContext& ctx) {
	createRoot("soldier", BTNode::EType::PRIORITY, nullptr, nullptr);
	addChild("soldier", "escape", BTNode::EType::ACTION, (BTCondition)&TCompAITest::conditionEscape, (BTAction)&TCompAITest::actionEscape);
	addChild("soldier", "combat", BTNode::EType::SEQUENCE, nullptr, nullptr);
	addChild("soldier", "idle", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAITest::actionIdle);
	addChild("combat", "pursuit", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAITest::actionPursuit);
	addChild("combat", "shoot", BTNode::EType::SEQUENCE, nullptr, nullptr);
	addChild("shoot", "shootgrenade", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAITest::actionShootGrenade);
	addChild("shoot", "shootpistol", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAITest::actionShootPistol);
}

void TCompAITest::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
	TCompName *tName = get<TCompName>();
	name = tName->getName();
}

int TCompAITest::actionIdle(float dt)
{
	dbg("%s: handling idle\n", name.c_str());
	return BTNode::ERes::LEAVE;
}

int TCompAITest::actionEscape(float dt)
{
	dbg("%s: handling escape\n", name.c_str());
	if (rand() % 5)
		return BTNode::ERes::LEAVE;
	return BTNode::ERes::STAY;
}

int TCompAITest::actionShoot(float dt)
{
	dbg("%s: handling shoot\n", name.c_str());
	return BTNode::ERes::LEAVE;
}

int TCompAITest::actionShootGrenade(float dt)
{
	dbg("%s: handling shootgrenade\n", name.c_str());
	return BTNode::ERes::LEAVE;
}

int TCompAITest::actionShootPistol(float dt)
{
	dbg("%s: handling shootpistol\n", name.c_str());
	return BTNode::ERes::LEAVE;
}

int TCompAITest::actionPursuit(float dt)
{
	dbg("%s: handling pursuit\n", name.c_str());
	return BTNode::ERes::LEAVE;
}

bool TCompAITest::conditionEscape(float dt)
{
	dbg("%s: testing escape\n", name.c_str());
	return rand() % 2;
}

void TCompAITest::registerMsgs()
{
	DECL_MSG(TCompAITest, TMsgEntityCreated, onMsgEntityCreated);
}




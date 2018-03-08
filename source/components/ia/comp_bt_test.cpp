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
	std::string name;
	std::string aux = "a";
	int foo = aux.compare("u");
	name = "john doe";
	for (int i = 0; i < name.size(); i++) {
		name.at(i) = toupper(name.at(i));
	}
	if (j.count("ai_test") > 0) {
		assert(j.count("createRoot") == 1);
		if (j.count("createRoot") == 1) {
			assert(j.count("rootName")==1);
			name = j.value("rootName","");
			assert(j.count("type") == 1);
			aux = j.value("type", "");
			int foo = aux.compare("PRIORITY");

		}
	}
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




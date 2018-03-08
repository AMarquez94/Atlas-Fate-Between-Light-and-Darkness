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

	std::string rootName, childName, type;
	BTNode::EType nodeType;

	assert(j.count("createRoot") == 1);
	if (j.count("createRoot") == 1) {
		auto& j_root = j["createRoot"];
		assert(j_root.count("rootName") == 1);
		rootName = j_root.value("rootName", "");
		assert(j_root.count("type") == 1);
		type = j_root.value("type", "");
		if (j_root.count("condition") == 1) {

		}
		if (j_root.count("action") == 1) {

		}
		ToUpperCase(type);
		nodeType = stringToNodeType(type);
		assert(nodeType != BTNode::EType::FAILURE);
		createRoot(rootName, nodeType, NULL, NULL);
	}
	//createRoot("soldier", BTNode::EType::PRIORITY, nullptr, nullptr);
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




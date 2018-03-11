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

	std::string rootName, childName, parentName, condition, action, type;
	BTNode::EType nodeType;
	BTAction actionNode;
	BTCondition conditionNode;

	loadActions();
	loadConditions();

	assert(j.count("createRoot") == 1);
	if (j.count("createRoot") == 1) {
		auto& j_root = j["createRoot"];
		assert(j_root.count("rootName") == 1);
		rootName = j_root.value("rootName", "");
		assert(j_root.count("type") == 1);
		type = j_root.value("type", "");
		ToUpperCase(type);
		nodeType = stringToNodeType(type);
		assert(nodeType != BTNode::EType::FAILURE);
		
		if (j_root.count("condition") == 1) {
			int a = 0;
		}
		if (j_root.count("action") == 1) {
			int a = 0;
		}

		createRoot(rootName, nodeType, NULL, NULL);
		int aux = j.count("addChild");
		if (j.count("addChild") > 0) {
			auto& j_addChild = j["addChild"];
			for (auto it = j_addChild.begin(); it != j_addChild.end(); ++it) {
				assert(it.value().count("parent") > 0);
				parentName = it.value()["parent"];
				assert(it.value().count("name") > 0);
				childName = it.value()["name"];
				assert(it.value().count("type") > 0);
				type = it.value()["type"];
				ToUpperCase(type);
				nodeType = stringToNodeType(type);
				assert(nodeType != BTNode::EType::FAILURE);
				assert(it.value().count("condition") > 0);
				condition = it.value()["condition"];
				//ToUpperCase(condition);
				conditionNode = conditions_initializer[condition];
				assert(it.value().count("action") > 0);
				action = it.value()["action"];
				//ToUpperCase(action);
				actionNode = actions_initializer[action];

				addChild(parentName, childName, nodeType, conditionNode, actionNode);
			}
		}
	}
}

void TCompAITest::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
	TCompName *tName = get<TCompName>();
	name = tName->getName();
}

void TCompAITest::loadActions() {
	actions_initializer.clear();

	actions_initializer["actionIdle"] = (BTAction)&TCompAITest::actionIdle;
	actions_initializer["actionEscape"] = (BTAction)&TCompAITest::actionEscape;
	actions_initializer["actionShoot"] = (BTAction)&TCompAITest::actionShoot;
	actions_initializer["actionShootGrenade"] = (BTAction)&TCompAITest::actionShootGrenade;
	actions_initializer["actionShootPistol"] = (BTAction)&TCompAITest::actionShootPistol;
	actions_initializer["actionPursuit"] = (BTAction)&TCompAITest::actionPursuit;
}

void TCompAITest::loadConditions() {
	conditions_initializer.clear();

	conditions_initializer["conditionEscape"] = (BTCondition)&TCompAITest::conditionEscape;
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




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
	BTAction actionNode = nullptr;
	BTCondition conditionNode = nullptr;

	loadActions();
	loadConditions();

	assert(j.count("createRoot") == 1);
	if (j.count("createRoot") == 1) {
		auto& j_root = j["createRoot"];
		assert(j_root.count("rootName") == 1);
		rootName = j_root.value("rootName", "defaultValue");
		assert(j_root.count("type") == 1);
		type = j_root.value("type", "defaultValue");
		ToUpperCase(type);
		nodeType = stringToNodeType(type);
		

		if (j_root.count("condition") == 1) {
			condition = j_root.value("condition", "defaultValue");
			if (conditions_initializer.find(condition) == conditions_initializer.end()) {
				conditionNode = nullptr;
			}
			else {
				conditionNode = conditions_initializer[condition];
			}
		}
		if (j_root.count("action") == 1) {
			action = j_root.value("action", "defaultValue");
			if (actions_initializer.find(action) == actions_initializer.end()) {
				actionNode = nullptr;
			}
			else {
				actionNode = actions_initializer[action];
			}
		}

		createRoot(rootName, nodeType, conditionNode, actionNode, nullptr);
		//int aux = j.count("addChild");
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

				assert(it.value().count("condition") > 0);
				condition = it.value()["condition"];
				//ToUpperCase(condition);
				if (conditions_initializer.find(condition) == conditions_initializer.end()) {
					conditionNode = nullptr;
					dbg("The condition %s of the child %s was not found. Setting it as NULL \n", condition.c_str(), childName.c_str());
				}
				else {
					conditionNode = conditions_initializer[condition];
					if (it.value().count("conditionArg") == 1) {
						auto& j_condArg = it.value()["conditionArg"];
						//We start retrieving the parameters
						if (j_condArg.count("escape") == 1) {
							conditionsArgs.escapeB = true;
							conditionsArgs.escape = j_condArg.value("escape", 0);
						}
					}
				}
				assert(it.value().count("action") > 0);
				action = it.value()["action"];
				//ToUpperCase(action);
				if (actions_initializer.find(action) == actions_initializer.end()) {
					actionNode = NULL;
					dbg("The action %s of the child %s was not found. Setting it as NULL \n", action.c_str(), childName.c_str());
				}
				else {
					actionNode = actions_initializer[action];
					if (it.value().count("actionArg") == 1) {
						auto& j_actArg = it.value()["actionArg"];
						//We start retrieving the parameters
						if (j_actArg.count("movement") == 1) {
							actionsArgs.movementB = true;
							actionsArgs.movement = j_actArg.value("movement", 0);
						}
						if (j_actArg.count("ugh") == 1) {
							actionsArgs.ughB = true;
							actionsArgs.ugh = j_actArg.value("ugh", 0);
						}
					}

				}
				addChild(parentName, childName, nodeType, conditionNode, actionNode, nullptr);
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
	if (conditionsArgs.escapeB) {
		dbg("%s: testing escape with parameter\n", name.c_str());
		return conditionsArgs.escape;
	}
	else {
		dbg("%s: testing escape\n", name.c_str());
		return rand() % 2;
	}
}

void TCompAITest::registerMsgs()
{
	DECL_MSG(TCompAITest, TMsgEntityCreated, onMsgEntityCreated);
}




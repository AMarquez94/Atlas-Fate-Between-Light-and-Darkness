#pragma once

#include "components/comp_base.h"
#include "btnode.h"

class TCompIAController : public TCompBase {

private:

	typedef int (TCompIAController::*BTAction)();
	typedef bool (TCompIAController::*BTCondition)();

	/* The nodes as map (so we have both map and tree for the same structure */
	std::map<std::string, BTNode *> tree;

	/* The cpp functions that implements node actions (behaviours) */
	std::map<std::string, BTAction> actions;

	/* The cpp functions that implements conditions */
	std::map<std::string, BTCondition> conditions;

	BTNode *root;
	BTNode *current;

	BTNode *createNode(std::string name);
	BTNode *findNode(std::string name);

public:

	void debugInMenu();
	virtual void load(const json& j, TEntityParseContext& ctx) = 0;

	/* Calls to declare root and children. Use nullptr when you dont need/want a BTCondition or BTAction */
	BTNode *createRoot(std::string rootName, BTNode::EType type, BTCondition btCondition, BTAction btAction);
	BTNode *addChild(std::string parentName, std::string childName, BTNode::EType type, BTCondition btCondition, BTAction btAction);

	void addAction(std::string actionName, BTAction btAction);
	int execAction(std::string actionName);
	void addCondition(std::string conditionName, BTCondition btCondition);
	bool testCondition(std::string conditionName);
	void setCurrent(BTNode *currentNode);

	void update(float dt);
};
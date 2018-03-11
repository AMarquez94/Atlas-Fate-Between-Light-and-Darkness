#pragma once

#include "components/comp_base.h"
#include "btnode.h"

class BTNode;

class TCompIAController : public TCompBase {


protected:

	typedef BTNode::ERes(TCompIAController::*BTAction)(float dt);
	typedef bool (TCompIAController::*BTCondition)(float dt);

	std::map<std::string, BTAction> actions_initializer;
	std::map<std::string, BTCondition> conditions_initializer;



private:

	/* The nodes as map (so we have both map and tree for the same structure */
	std::map<std::string, BTNode *> tree;

	/* The cpp functions that implements node actions (behaviours) */
	std::map<std::string, BTAction> actions;


	/* The cpp functions that implements conditions */
	std::map<std::string, BTCondition> conditions;


	BTNode *root;
	BTNode *current;

	BTNode *createNode(const std::string& name);
	BTNode *findNode(const std::string& name);

public:

	std::string name;

	void debugInMenu();
	virtual void load(const json& j, TEntityParseContext& ctx) = 0;

	/* Calls to declare root and children. Use nullptr when you dont need/want a BTCondition or BTAction */
	BTNode *createRoot(const std::string& rootName, BTNode::EType type, BTCondition btCondition, BTAction btAction);
	BTNode *addChild(const std::string& parentName, std::string childName, BTNode::EType type, BTCondition btCondition, BTAction btAction);

	void addAction(const std::string& actionName, BTAction btAction);
	BTNode::ERes execAction(const std::string& actionName, float dt);
	void addCondition(const std::string& conditionName, BTCondition btCondition);
	bool testCondition(const std::string& conditionName, float dt);
	void setCurrent(BTNode *currentNode);

	void update(float dt);

	BTNode::EType stringToNodeType(std::string&);
	virtual void loadActions() { }
	virtual void loadConditions() { }
};
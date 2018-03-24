#pragma once

#include "components/comp_base.h"
#include "btnode.h"
#include "utils\variant.h"

class BTNode;

class TCompIAController : public TCompBase {


protected:

	typedef BTNode::ERes(TCompIAController::*BTAction)(float dt);
	typedef bool (TCompIAController::*BTCondition)(float dt);
	typedef bool (TCompIAController::*BTAssert)(float dt);
	
	//Auxiliar maps for initializing in the instantiations
	std::map<std::string, BTAction> actions_initializer;
	std::map<std::string, BTCondition> conditions_initializer;
	std::map<std::string, BTCondition> asserts_initializer;
	std::map<std::string, CVariant> arguments;

	
	//Auxiliar functions for the load in the instantiations
	BTNode::EType stringToNodeType(std::string&);
	virtual void loadActions() {};
	virtual void loadConditions() {};
	virtual void loadAsserts() {};
	//Function to use inside each instantiation
	virtual void loadArguments() {};

	void loadTree(const json& j);

	BTNode *current;

private:

	/* The nodes as map (so we have both map and tree for the same structure */
	std::map<std::string, BTNode *> tree;

	/* The cpp functions that implements node actions (behaviours) */
	std::map<std::string, BTAction> actions;

	/* The cpp functions that implements conditions */
	std::map<std::string, BTCondition> conditions;

	/* The cpp functions that implements assert conditions */
	std::map<std::string, BTAssert> asserts;

	BTNode *root;

	BTNode *createNode(const std::string& name);
	BTNode *findNode(const std::string& name);

	void printTree();

public:

	std::string name;

	void debugInMenu();
	virtual void load(const json& j, TEntityParseContext& ctx) {};

	/* Calls to declare root and children. Use nullptr when you dont need/want a BTCondition or BTAction */
	BTNode *createRoot(const std::string& rootName, BTNode::EType type, BTCondition btCondition, BTAction btAction, BTAssert btAssert);
	BTNode *addChild(const std::string& parentName, std::string childName, BTNode::EType type, BTCondition btCondition, BTAction btAction, BTAssert btAssert);

	void addAction(const std::string& actionName, BTAction btAction);
	BTNode::ERes execAction(const std::string& actionName, float dt);
	void addCondition(const std::string& conditionName, BTCondition btCondition);
	bool testCondition(const std::string& conditionName, float dt);
	void addAssert(const std::string& assertName, BTAssert btAssert);
	bool testAssert(const std::string& assertName, float dt);
	void setCurrent(BTNode *currentNode);
	BTNode* getCurrent() { return current; };

	void update(float dt);
};
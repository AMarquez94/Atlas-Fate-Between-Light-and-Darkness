#pragma once

#include "components/comp_base.h"
#include "btnode.h"
#include "utils\variant.h"

class BTNode;

struct HistoricalAction {
  const char* action;
  int number_of_times;
};

class TCompIAController : public TCompBase {


private:

  /* TODO: Temp - Borrar. Solo con motivo de debug */
  std::vector<HistoricalAction> historic;

protected:

    bool pausedAI = false;

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

	void loadTree(const json& json);
	void loadParameters(const json& j);
	void loadParameterVariables(const json& j, const std::string& type, const std::string& name);

	BTNode *current;

	void onMsgScenePaused(const TMsgScenePaused& msg);
	void onMsgAIPaused(const TMsgAIPaused& msg);

	bool isNodeSonOf(BTNode * son, const std::string& possibleParent);
  bool isNodeName(BTNode * node, const std::string& possibleName);

  CHandle myHandle;

protected:

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

	enum BTType { PATROL = 0, MIMETIC, NUM_ENEMIES };

	std::string name;
	BTType btType;
	float timeAnimating = 0.0f;

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

	void setCurrentByName(const std::string& stateName);

	void update(float dt);
  virtual void preUpdate(float dt) = 0;
  virtual void postUpdate(float dt) = 0;

  /* TODO: remove - solo para debug */
  void addActionToHistoric(const std::string& action);
};
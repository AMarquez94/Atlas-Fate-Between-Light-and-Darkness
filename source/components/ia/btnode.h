#pragma once

#include "components/comp_base.h"
//#include "comp_bt.h"

class TCompIAController;

class BTNode {

public:
	enum EType { RANDOM = 0, SEQUENCE, PRIORITY, ACTION, NUM_TYPES};
	enum ERes { STAY = 0, LEAVE, NUM_RES, UNIMPLEMENTED };

	BTNode(const std::string& btName);
	void create(const std::string& btName);
	bool isRoot();
	void setParent(BTNode *newParent);
	BTNode * getParent() { return parent; };
	void setRight(BTNode *newRight);
	void addChild(BTNode *newChild);
	void setType(EType type);
	void update(float dt, TCompIAController* bt);
	const std::string getName();
	void printNode(int level, TCompIAController *bt);

private:

	std::string name;
	EType type;
	std::vector<BTNode*> children;
	BTNode *parent;
	BTNode *right;
};
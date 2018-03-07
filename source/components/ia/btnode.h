#pragma once

#include "components/comp_base.h"
#include "comp_bt.h"

struct BTNode {

private: 

	enum EType;

	std::string name;
	EType type;
	std::vector<BTNode*> children;
	BTNode *parent;
	BTNode *right;

public:
	enum EType { RANDOM = 0, SEQUENCE, PRIORITY, ACTION, NUM_TYPES };
	enum ERes { STAY = 0, LEAVE, NUM_RES};

	BTNode(std::string btName);
	void create(std::string btName);
	bool isRoot();
	void setParent(BTNode *newParent);
	void setRight(BTNode *newRight);
	void addChild(BTNode *newChild);
	void setType(EType type);
	void update(float dt, TCompIAController* bt);
	const std::string getName();

};
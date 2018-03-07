#include "mcv_platform.h"
#include "comp_bt.h"

BTNode * TCompIAController::createNode(std::string name)
{
	assert(findNode(name) != nullptr);
	BTNode *btNode = new BTNode(name);
	tree[name] = btNode;
	return btNode;
}

BTNode * TCompIAController::findNode(std::string name)
{
	return tree.find(name) == tree.end() ? nullptr : tree[name];
}

void TCompIAController::debugInMenu() {

}

BTNode * TCompIAController::createRoot(std::string rootName, BTNode::EType type, BTCondition btCondition, BTAction btAction)
{
	BTNode *root = createNode(rootName);
	root->setParent(nullptr);
	this->root = root;
	root->setType(type);
	if (btCondition != nullptr) {
		addCondition(rootName, btCondition);
	}
	if (btAction != nullptr) {
		addAction(rootName, btAction);
	}

	current = nullptr;
	return root;
}

BTNode * TCompIAController::addChild(std::string parentName, std::string childName, BTNode::EType type, BTCondition btCondition, BTAction btAction)
{
	BTNode *parent = findNode(parentName);
	assert(parent);
	BTNode *son = createNode(childName);
	parent->addChild(son);
	son->setParent(parent);
	son->setType(type);
	if (btCondition != nullptr) {
		addCondition(childName, btCondition);
	}
	if (btAction != nullptr) {
		addAction(childName, btAction);
	}
	return son;
}

void TCompIAController::addAction(std::string actionName, BTAction btAction)
{
	assert(actions.find(actionName) == actions.end());
	actions[actionName] = btAction;	//TODO: mirar fallo
}

int TCompIAController::execAction(std::string actionName)
{
	if (actions.find(actionName) == actions.end()) {
		fatal("ERROR: Missing node action for node %s\n", actionName);
		return BTNode::LEAVE;
	}
	return (this->*actions[actionName])();
}

void TCompIAController::addCondition(std::string conditionName, BTCondition btCondition)
{
	assert(conditions.find(conditionName) == conditions.end());
	conditions[conditionName] = btCondition;
}

bool TCompIAController::testCondition(std::string conditionName)
{
	if (conditions.find(conditionName) == conditions.end()) {
		//No condition => we assume is true
		return true;
	}
	else {
		return (this->*conditions[conditionName])();
	}
}

void TCompIAController::setCurrent(BTNode * currentNode)
{
	current = currentNode;
}

void TCompIAController::update(float dt) {
	if (current == nullptr) {
		root->update(dt, this);
	}
	else {
		current->update(dt, this);
	}
}


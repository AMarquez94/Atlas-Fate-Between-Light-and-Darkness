#include "mcv_platform.h"
#include "comp_bt.h"

void TCompIAController::debugInMenu() {}

BTNode * TCompIAController::createNode(const std::string& name)
{
	assert(findNode(name) == nullptr);
	BTNode *btNode = new BTNode(name);
	tree[name] = btNode;
	return btNode;
}

BTNode * TCompIAController::findNode(const std::string& name)
{
	return tree.find(name) == tree.end() ? nullptr : tree[name];
}

BTNode * TCompIAController::createRoot(const std::string& rootName, BTNode::EType type, BTCondition btCondition, BTAction btAction)
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

BTNode * TCompIAController::addChild(const std::string& parentName, std::string childName, BTNode::EType type, BTCondition btCondition, BTAction btAction)
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

void TCompIAController::addAction(const std::string& actionName, BTAction btAction)
{
	assert(actions.find(actionName) == actions.end());
	actions[actionName] = btAction;	//TODO: mirar fallo
}

BTNode::ERes TCompIAController::execAction(const std::string& actionName, float dt)
{
	if (actions.find(actionName) == actions.end()) {
		fatal("ERROR: Missing node action for node %s\n", actionName);
		return BTNode::LEAVE;
	}
	return (this->*actions[actionName])(dt);
}

void TCompIAController::addCondition(const std::string& conditionName, BTCondition btCondition)
{
	assert(conditions.find(conditionName) == conditions.end());
	conditions[conditionName] = btCondition;
}

bool TCompIAController::testCondition(const std::string& conditionName, float dt)
{
	if (conditions.find(conditionName) == conditions.end()) {
		//No condition => we assume is true
		return true;
	}
	else {
		return (this->*conditions[conditionName])(dt);
	}
}

void TCompIAController::setCurrent(BTNode * currentNode)
{
	current = currentNode;
}

BTNode::EType TCompIAController::stringToNodeType(std::string& string) {
	if (string.compare("RANDOM") == 0) {
		return BTNode::EType::RANDOM;
	}
	else if (string.compare("SEQUENCE") == 0) {
		return BTNode::EType::SEQUENCE;
	}
	else if (string.compare("PRIORITY") == 0) {
		return BTNode::EType::PRIORITY;
	}
	else if (string.compare("ACTION") == 0) {
		return BTNode::EType::ACTION;
	}
	else {
		return BTNode::EType::FAILURE;
	}
}

void TCompIAController::update(float dt) {
	if (current == nullptr) {
		root->update(dt, this);
	}
	else {
		current->update(dt, this);
	}
}
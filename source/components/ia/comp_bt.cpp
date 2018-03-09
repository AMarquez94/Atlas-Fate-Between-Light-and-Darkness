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

BTNode * TCompIAController::createRoot(const std::string& rootName, BTNode::EType type, BTCondition btCondition, BTAction btAction, BTAssert btAssert)
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
	if (btAssert != nullptr) {
		addAssert(rootName, btAssert);
	}

	current = nullptr;
	return root;
}

BTNode * TCompIAController::addChild(const std::string& parentName, std::string childName, BTNode::EType type, BTCondition btCondition, BTAction btAction, BTAssert btAssert)
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
	if (btAssert != nullptr) {
		addAssert(childName, btAssert);
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

void TCompIAController::addAssert(const std::string & assertName, BTAssert btAssert)
{
	assert(asserts.find(assertName) == asserts.end());
	asserts[assertName] = btAssert;
}

bool TCompIAController::testAssert(const std::string & assertName, float dt)
{
	if (asserts.find(assertName) == asserts.end()) {
		//No assert condition => we assume is true
		return true;
	}
	else {
		return (this->*asserts[assertName])(dt);
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
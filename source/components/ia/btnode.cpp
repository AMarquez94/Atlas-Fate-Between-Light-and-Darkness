#include "mcv_platform.h"
#include "btnode.h"

BTNode::BTNode(std::string btName)
{
	this->name = btName;
}

void BTNode::create(std::string btName) {
	this->name = btName;
}

bool BTNode::isRoot() {
	return this->parent == nullptr;	//NULL or nullptr?
}

void BTNode::setParent(BTNode *newParent) {
	this->parent = newParent;
}

void BTNode::setRight(BTNode *newRight) {
	this->right = newRight;
}

void BTNode::addChild(BTNode *newChild) {
	if (!this->children.empty()) {

		/* If node already had children, last node connected to the new one */
		children.back()->setRight(newChild);
	}
	children.push_back(newChild);
	newChild->right = nullptr;
}

void BTNode::setType(EType type) {
	this->type = type;
}

void BTNode::update(float dt, TCompIAController* bt) {

}

const std::string BTNode::getName() {
	return this->name;
}

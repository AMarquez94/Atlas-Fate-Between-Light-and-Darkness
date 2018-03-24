#include "mcv_platform.h"
#include "comp_bt.h"

void TCompIAController::debugInMenu() {

	if (ImGui::CollapsingHeader("BT Nodes")) {
		ImGui::Separator();
		printTree();
		ImGui::Separator();
	}
}

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
		fatal("ERROR: Missing node action for action%s \n", actionName.c_str());
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
		fatal("Node type %s not recognized \n", string.c_str());
		return BTNode::EType::NUM_TYPES;
	}
}

void TCompIAController::loadTree(const json & j)
{
	std::string rootName, childName, parentName, condition, action, assert, type, variantName, variantType, variantOp;

	BTNode::EType nodeType;
	BTAction actionNode = nullptr;
	BTCondition conditionNode = nullptr, assertNode = nullptr;

	assert(j.count("createRoot") == 1);
	if (j.count("createRoot") == 1) {
		auto& j_root = j["createRoot"];
		assert(j_root.count("rootName") == 1);
		rootName = j_root.value("rootName", "defaultValue");
		assert(j_root.count("type") == 1);
		type = j_root.value("type", "defaultValue");
		ToUpperCase(type);
		nodeType = stringToNodeType(type);


		if (j_root.count("condition") == 1) {
			condition = j_root.value("condition", "defaultValue");
			if (conditions_initializer.find(condition) == conditions_initializer.end()) {
				conditionNode = nullptr;
			}
			else {
				conditionNode = conditions_initializer[condition];
			}
		}
		if (j_root.count("action") == 1) {
			action = j_root.value("action", "defaultValue");
			if (actions_initializer.find(action) == actions_initializer.end()) {
				actionNode = nullptr;
			}
			else {
				actionNode = actions_initializer[action];
			}
		}
		if (j_root.count("assert") == 1) {
			assert = j_root.value("assert", "defaultValue");
			if (asserts_initializer.find(assert) == asserts_initializer.end()) {
				assertNode = nullptr;
			}
			else {
				assertNode = asserts_initializer[assert];
			}
		}

		createRoot(rootName, nodeType, conditionNode, actionNode, assertNode);
		if (j.count("addChild") > 0) {
			auto& j_addChild = j["addChild"];
			for (auto it = j_addChild.begin(); it != j_addChild.end(); ++it) {
				assert(it.value().count("parent") > 0);
				parentName = it.value().value("parent", "");
				assert(it.value().count("name") > 0);
				childName = it.value().value("name", "");
				assert(it.value().count("type") > 0);
				type = it.value().value("type", "");
				ToUpperCase(type);
				nodeType = stringToNodeType(type);

				assert(it.value().count("condition") > 0);
				condition = it.value().value("condition", "");
				//ToUpperCase(condition);
				if (conditions_initializer.find(condition) == conditions_initializer.end()) {
					conditionNode = nullptr;
					//dbg("The condition %s of the child %s was not found. Setting it as NULL \n", condition.c_str(), childName.c_str());
				}
				else {
					conditionNode = conditions_initializer[condition];
					if (it.value().count("conditionArg") > 0) {
						auto& j_conditionArg = it.value()["conditionArg"];
						for (auto ite = j_conditionArg.begin(); ite != j_conditionArg.end(); ++ite) {
							assert(ite.value().count("variable_name") > 0);
							assert(ite.value().count("variable_type") > 0);
							assert(ite.value().count("variable_value") > 0);
							assert(ite.value().count("variable_op") > 0);

							variantName = ite.value()["variable_name"];
							variantType = ite.value()["variable_type"];
							ToUpperCase(variantType);
							variantOp = ite.value()["variable_op"];

							if (variantType == "INT") {
								int variantValue = ite.value()["variable_value"];
								CVariant parameter;
								parameter.setInt(variantValue);
								arguments[variantName] = parameter;
							}
							else if (variantType == "FLOAT") {
								float variantValue = ite.value()["variable_value"];
								CVariant parameter;
								parameter.setFloat(variantValue);
								arguments[variantName] = parameter;
							}
							else if (variantType == "BOOL") {
								bool variantValue = ite.value()["variable_value"];
								CVariant parameter;
								parameter.setBool(variantValue);
								arguments[variantName] = parameter;
							}
							else {
								fatal("La vida es dura, el tipo de variable %s no es aceptada. Revisa el Json", variantType.c_str());
							}
						}

					}
				}
				assert(it.value().count("action") > 0);
				action = it.value().value("action", "");
				//ToUpperCase(action);
				if (actions_initializer.find(action) == actions_initializer.end()) {
					actionNode = NULL;
					//dbg("The action %s of the child %s was not found. Setting it as NULL \n", action.c_str(), childName.c_str());
				}
				else {
					actionNode = actions_initializer[action];
					if (it.value().count("actionArg") > 0) {
						auto& j_actionArg = it.value()["actionArg"];
						for (auto ite = j_actionArg.begin(); ite != j_actionArg.end(); ++ite) {
							assert(ite.value().count("variable_name") > 0);
							assert(ite.value().count("variable_type") > 0);
							assert(ite.value().count("variable_value") > 0);
							assert(ite.value().count("variable_op") > 0);

							variantName = ite.value()["variable_name"];
							variantType = ite.value()["variable_type"];
							ToUpperCase(variantType);
							variantOp = ite.value()["variable_op"];

							if (variantType == "INT") {
								int variantValue = ite.value()["variable_value"];
								CVariant parameter;
								parameter.setInt(variantValue);
								arguments[variantName] = parameter;
							}
							else if (variantType == "FLOAT") {
								float variantValue = ite.value()["variable_value"];
								CVariant parameter;
								parameter.setFloat(variantValue);
								arguments[variantName] = parameter;
							}
							else if (variantType == "BOOL") {
								bool variantValue = ite.value()["variable_value"];
								CVariant parameter;
								parameter.setBool(variantValue);
								arguments[variantName] = parameter;
							}
							else {
								fatal("La vida es dura, el tipo de variable %s no es aceptada. Revisa el Json", variantType.c_str());
							}
						}

					}
				}
				assert(it.value().count("assert") > 0);
				assert = it.value().value("assert", "");
				//ToUpperCase(condition);
				if (asserts_initializer.find(assert) == asserts_initializer.end()) {
					assertNode = nullptr;
					//dbg("The assert %s of the child %s was not found. Setting it as NULL \n", assert.c_str(), childName.c_str());
				}
				else {
					assertNode = asserts_initializer[assert];
				}
				addChild(parentName, childName, nodeType, conditionNode, actionNode, assertNode);
			}
		}
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


void TCompIAController::printTree()
{
	root->printNode(0, this);
}

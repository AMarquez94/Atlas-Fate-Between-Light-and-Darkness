#include "mcv_platform.h"
#include "comp_bt.h"
#include "resources/json_resource.h"

void TCompIAController::debugInMenu() {

	if (ImGui::CollapsingHeader("BT Nodes")) {
		ImGui::Separator();
		printTree();
		ImGui::Separator();
	}

  if (ImGui::CollapsingHeader("Action Historic")) {
    ImGui::Separator();
    for (int i = 0; i < historic.size(); i++) {
      ImGui::Text("Action: %s - %d", historic[i].action, historic[i].number_of_times);
    }
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

void TCompIAController::loadTree(const json & json_bt)
{
  if (json_bt.count("bt") <= 0){

    /* Only load if there is a bt */
    return;
  }
  const json& j = Resources.get(json_bt["bt"])->as<CJsonResource>()->getJson();

	std::string rootName, childName, parentName, condition, action, assert, type;

	BTNode::EType nodeType;
	BTAction actionNode = nullptr;
	BTCondition conditionNode = nullptr, assertNode = nullptr;

	//assert(j.count("root") == 1);
	if (j.count("root") == 1) {
		auto& j_root = j["root"];
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
		if (j.count("children") > 0) {
			auto& j_addChild = j["children"];
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
						//Default parameters for the conditions
						//Retrieving values from the prefab
						auto& j_conditionArg = it.value()["conditionArg"];
						loadParameterVariables(j_conditionArg, condition, childName);
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
						//Default parameters for the actions
						//Retrieving values from the prefab
						auto& j_actionArg = it.value()["actionArg"];
						loadParameterVariables(j_actionArg, action, childName);
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
					if (it.value().count("assertArg") > 0) {
						//Default parameters for the conditions
						//Retrieving values from the prefab
						auto& j_assertArg = it.value()["assertArg"];
						loadParameterVariables(j_assertArg, assert, childName);
					}
				}
				addChild(parentName, childName, nodeType, conditionNode, actionNode, assertNode);
			}
		}
	}
}

void TCompIAController::loadParameters(const json& j) {
	
	std::string argCondName, argActName, aux;
	CVariant::EType type;

	if (j.count("argCond") > 0) {
		auto& j_argCond = j["argCond"];
		for (auto it = j_argCond.begin(); it != j_argCond.end(); ++it) {
			assert(it.value().count("nodeName") == 1);
			assert(it.value().count("conditionName") == 1);
			assert(it.value().count("variableName") == 1);
			assert(it.value().count("newValue") == 1);

			aux = it.value().value("conditionName", "");

			argCondName = it.value().value("variableName", "");
			argCondName += "_";
			argCondName += aux;
			argCondName += "_";
			aux = it.value().value("nodeName", "");
			argCondName += aux;

			assert(arguments.find(argCondName) != arguments.end());
			type = arguments[argCondName].getType();
			switch (type) {
			case CVariant::EType::BOOL:
			{
				arguments[argCondName].setBool(it.value()["newValue"]);

				break;
			}
			case CVariant::EType::FLOAT:
			{
				arguments[argCondName].setFloat(it.value()["newValue"]);

				break;
			}
			case CVariant::EType::INT:
			{
				arguments[argCondName].setInt(it.value()["newValue"]);

				break;
			}
			case CVariant::EType::STRING:
			{
				arguments[argCondName].setString(it.value()["newValue"]);

				break;
			}
			default:
			{
				fatal("Type not defined!");
				break;
			}
			}
		}
	}
	if (j.count("argAct") > 0) {
		auto& j_argAct = j["argAct"];
		for (auto it = j_argAct.begin(); it != j_argAct.end(); ++it) {
			assert(it.value().count("nodeName") == 1);
			assert(it.value().count("actionName") == 1);
			assert(it.value().count("variableName") == 1);
			assert(it.value().count("newValue") == 1);

			aux = it.value().value("actionName", "");

			argActName = it.value().value("variableName", "");
			argActName += "_";
			argActName += aux;
			argActName += "_";
			aux = it.value().value("nodeName", "");
			argActName += aux;

			assert(arguments.find(argActName) != arguments.end());
			type = arguments[argActName].getType();
			switch (type) {
			case CVariant::EType::BOOL:
			{
				arguments[argCondName].setBool(it.value()["newValue"]);

				break;
			}
			case CVariant::EType::FLOAT:
			{
				arguments[argCondName].setFloat(it.value()["newValue"]);

				break;
			}
			case CVariant::EType::INT:
			{
				arguments[argCondName].setInt(it.value()["newValue"]);

				break;
			}
			case CVariant::EType::STRING:
			{
				arguments[argCondName].setString(it.value()["newValue"]);

				break;
			}
			default:
			{
				fatal("Type not defined!");
				break;
			}
			}
		}
	}
}

void TCompIAController::loadParameterVariables(const json & j,const std::string& type,const std::string& name)
{
	std::string variantName, variantType, variantOp;
	for (auto ite = j.begin(); ite != j.end(); ++ite) {
		//Running some checks
		assert(ite.value().count("variable_name") > 0);
		assert(ite.value().count("variable_type") > 0);
		assert(ite.value().count("variable_value") > 0);
		assert(ite.value().count("variable_op") > 0);
		//Retrieving info
		variantName = ite.value().value("variable_name", "");
		variantName = variantName + "_" + type + "_" + name;
		variantType = ite.value().value("variable_type", "");
		ToUpperCase(variantType);
		variantOp = ite.value().value("variable_op", "");
		//Depending on the variable type, we create the variant and we save it in arguments
		if (variantType == "INT") {
			assert(ite.value()["variable_value"].is_number_integer());
			int variantValue = ite.value()["variable_value"];
			CVariant parameter;
			parameter.setInt(variantValue);
			arguments[variantName] = parameter;
		}
		else if (variantType == "FLOAT") {
			assert(ite.value()["variable_value"].is_number_float());
			float variantValue = ite.value()["variable_value"];
			CVariant parameter;
			parameter.setFloat(variantValue);
			arguments[variantName] = parameter;
		}
		else if (variantType == "BOOL") {
			assert(ite.value()["variable_value"].is_boolean());
			bool variantValue = ite.value()["variable_value"];
			CVariant parameter;
			parameter.setBool(variantValue);
			arguments[variantName] = parameter;
		}
		else if (variantType == "STRING") {
			assert(ite.value()["variable_value"].is_string());
			std::string variantValue = ite.value()["variable_value"];
			CVariant parameter;
			parameter.setString(variantValue);
			arguments[variantName] = parameter;
		}
		else {
			fatal("La vida es dura, el tipo de variable %s no es aceptada. Revisa el Json", variantType.c_str());
		}
	}
}

void TCompIAController::setCurrentByName(const std::string & stateName)
{
	current = findNode(stateName);
}

void TCompIAController::update(float dt) {
	if (!paused && !pausedAI && myHandle.getOwner().isValid()) {
    preUpdate(dt);
		if (current == nullptr) {
			root->update(dt, this);
		}
		else {
			current->update(dt, this);
		}
    postUpdate(dt);
	}
}

void TCompIAController::printTree()
{
	root->printNode(0, this);
}

void TCompIAController::onMsgScenePaused(const TMsgScenePaused & msg)
{
	paused = !paused;
}

void TCompIAController::onMsgAIPaused(const TMsgAIPaused & msg)
{
    pausedAI = !pausedAI;
}

bool TCompIAController::isNodeName(BTNode * node, const std::string & possibleName)
{
  return node != nullptr && node->getName().compare(possibleName) == 0;
}

bool TCompIAController::isNodeSonOf(BTNode * son, const std::string& possibleParent)
{
	assert(findNode(possibleParent));
	if (son == nullptr || son->isRoot()) {
		return false;
	}
	else {
		if (son->getParent()->getName().compare(possibleParent) == 0) {
			return true;
		}
		else {
			return isNodeSonOf(son->getParent(), possibleParent);
		}
	}
}

void TCompIAController::addActionToHistoric(const std::string & action)
{
  if (historic.size() == 0 || std::strcmp(historic[historic.size() - 1].action, action.c_str()) != 0) {
    HistoricalAction historicalAction;
    historicalAction.action = action.c_str();
    historicalAction.number_of_times = 1;
		historic.push_back(historicalAction);
		if (historic.size() >= 500) {
			historic.erase(historic.begin());
		}
  }
  else {
    historic[historic.size() - 1].number_of_times += 1;
  }
}

#pragma once

#include "components/comp_base.h"
#include "comp_bt_enemy.h"
#include "modules/module_ia.h"

class TCompAIPlayer : public TCompIAController {

private:

	float _speed, _rotationSpeed;
	std::vector<Waypoint> _waypoints;
	int _currentWaypoint;



	void onMsgPlayerAIEnabled(const TMsgPlayerAIEnabled& msg);
	void onMsgEntityCreated(const TMsgEntityCreated& msg);

	//load
	void loadActions() override;
	void loadConditions() override;
	void loadAsserts() override;

    DECL_SIBLING_ACCESS();

public:
	
	bool enabledPlayerAI;
    
	void load(const json& j, TEntityParseContext& ctx) override;
	void preUpdate(float dt) override;
	void postUpdate(float dt) override;
    void debugInMenu();

	static void registerMsgs();

	BTNode::ERes actionGoToWpt(float dt);

	bool conditionHasBeenEnabled(float dt);


	//Auxiliar
	bool move(float dt);
};
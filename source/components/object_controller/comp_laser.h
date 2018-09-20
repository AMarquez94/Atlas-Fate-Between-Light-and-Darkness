#pragma once

#include "components/comp_base.h"

class TCompLaser : public TCompBase {

	DECL_SIBLING_ACCESS();

	physx::PxQueryFilterData laserFilter;
	bool playerDetected = false;
	std::string parent_name;
	CHandle h_parent;

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	static void registerMsgs();
	bool isPlayerOnLaser();

	void onEntityCreated(const TMsgEntitiesGroupCreated& msg);
};
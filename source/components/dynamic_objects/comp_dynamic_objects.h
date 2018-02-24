#pragma once

#include "components\comp_base.h"



class TCompDynamicObjects : public TCompBase {
	DECL_SIBLING_ACCESS();
	
	/* Atributes */
	std::vector<VEC3> _waypoints;
	int _currentWaypoint;
	float _speed;
	float _rotationSpeed;
	bool _start = false;

	void rotateTowardsVec(VEC3 objective, float dt);

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void Init();
	void update(float dt);
	void StartCycle();

	void addWaypoint(VEC3 wpt) { _waypoints.push_back(wpt); };
	VEC3 getWaypoint() { return _waypoints[_currentWaypoint]; }
	std::vector<VEC3> getWaypoints() { return _waypoints; }

};

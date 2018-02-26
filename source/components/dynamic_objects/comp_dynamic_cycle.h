//#pragma once
//
//#include "components\comp_base.h"
//
////This components only works with two waypoints so far.
////TO_DO: Support for more than two waypoints
//
//class TCompDynamicCycle : public TCompBase {
//	DECL_SIBLING_ACCESS();
//	std::vector <VEC3> _waypoints;
//	int _currentWaypoint = 0;
//	float _total = 0.0f;
//	float _offset = 1.0f;
//	float _width = 2.0f;
//	int _numClones;
//	float _amountMoved = 0.0f;
//	std::vector <VEC3> _pos[];
//
//	void addWaypoint(VEC3 wpt) { _waypoints.push_back(wpt); };
//	VEC3 getWaypoint() { return _waypoints[_currentWaypoint]; }
//public:
//	void debugInMenu();
//	void update(float dt);
//	void Init();
//	void CalculatePositions(int);
//	void CreateClones();
//};
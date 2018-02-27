#pragma once

#include "components\comp_base.h"



class TCompDynamicObjects : public TCompBase {
	DECL_SIBLING_ACCESS();
	
	/* Atributes */
	std::vector<VEC3> _waypoints;
	//Current Waypoint
	int _currentWaypoint;
	//Speed
	float _speed;
	//Rotation Speed
	float _rotationSpeed;
	//Can we start?
	bool _start = false;
	//Total distance
	float _totalDistance = 0.0f;
	//Space inbetween objects
	float _offset = 5.0f;
	//Width of the capsules hardcoded TO-DO: Get this from the collider
	float _width = 2.0f;
	//Number of clones to be created
	int _numClones;
	//Amount of the instance moved
	float _amountMoved = 0.0f;
	//Direction vector
	VEC3 _dir;
	//Positions of each instance
	std::vector <VEC3> _pos;
	//Clones
	std::vector <CEntity*> _clones;

	void rotateTowardsVec(VEC3 objective, float dt);

public:
	void debugInMenu();
	//Retrieve waypoints, speed, rotation speed
	void load(const json& j, TEntityParseContext& ctx); 
	//Initialize information
	void Init(); 
	void update(float dt);
	void StartCycle();
	void CalculatePositions(int);
	void CreateClone(TEntityParseContext, int);
	void Move(TCompTransform*, float);


	void addWaypoint(VEC3 wpt) { _waypoints.push_back(wpt); };
	VEC3 getWaypoint() { return _waypoints[_currentWaypoint]; }
	std::vector<VEC3> getWaypoints() { return _waypoints; }

};

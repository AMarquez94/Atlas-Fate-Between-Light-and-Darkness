#include "mcv_platform.h"
#include "comp_dynamic_cycle.h"
#include "comp_dynamic_objects.h"

DECL_OBJ_MANAGER("dynamic_cycle", TCompDynamicCycle);

void TCompDynamicCycle::debugInMenu() {

}

void TCompDynamicCycle::update(float dt) {
	//Init();
	//CreateClones();

}

void TCompDynamicCycle::Init() {
	
	TCompDynamicObjects *foo = get<TCompDynamicObjects>();
	//Initialization of _waypoints from TCompDynamicObjects
	for (auto it = 0; it <= foo->getWaypoints().size()-1; it++) {
		_waypoints.push_back(foo->getWaypoints()[it]);
	}
	for (auto it = 0; it < _waypoints.size() - 1; it++) {
		_total += VEC3::Distance(_waypoints[it], _waypoints[it+1]);
	}

	_numClones = _total / (_width + _offset);
}

void TCompDynamicCycle::CreateClones() {
	VEC3 dir = _waypoints[1] - _waypoints[0];
	_waypoint[0] + 
}

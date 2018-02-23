#include "mcv_platform.h"
#include "comp_dynamic_objects.h"
#include "comp_transform.h"
#include "entity/entity_parser.h"
#include "render/render_utils.h"


DECL_OBJ_MANAGER("dynamic_objects", TCompDynamicObjects);


void TCompDynamicObjects::debugInMenu() {
	TCompTransform *c_my_transform = get<TCompTransform>();
	VEC3 Pos = c_my_transform->getPosition();
	ImGui::DragFloat("Point", &Pos.x, 0.1f, -20.f, 20.f);
	ImGui::DragFloat("Point", &Pos.y, 0.1f, -20.f, 20.f);
	ImGui::DragFloat("Point", &Pos.z, 0.1f, -20.f, 20.f);

	//Showing Waypoints
	if (ImGui::TreeNode("Waypoints")) {
		for (auto& v : _waypoints) {
			ImGui::PushID(&v);
			ImGui::DragFloat3("Point", &v.x, 0.1f, -20.f, 20.f);
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
	ImGui::DragFloat("Speed", &_speed, 0.1f, 0.f, 10.f);
	ImGui::DragInt("Current Waypoint", &_currentWaypoint, 0, 0, 10);
}


void TCompDynamicObjects::load(const json& j, TEntityParseContext& ctx) {
	Init();
	//int a = j.count("waypoints");
	assert(j.count("waypoints") >= 1);
	auto& j_waypoints = j["waypoints"];
	for (auto it = j_waypoints.begin(); it != j_waypoints.end(); ++it) {

		VEC3 wpt;
		assert(it.value().count("position") == 1);

		wpt = loadVEC3(it.value()["position"]);
		
		addWaypoint(wpt);
	}
	_speed = j.value("speed", 2.0f);
	_rotationSpeed = deg2rad(j.value("rotationSpeed", 90));


}

void TCompDynamicObjects::Init() {
	_currentWaypoint = 0;
}

void TCompDynamicObjects::rotateTowardsVec(VEC3 objective, float dt) {
	/*TCompTransform *c_my_transform = get<TCompTransform>();	float y, r, p;
	c_my_transform->getYawPitchRoll(&y, &p, &r);
	float deltaYaw = c_my_transform->getDeltaYawToAimTo(objective);
	y += deltaYaw;
	c_my_transform->setYawPitchRoll(y, p, r);*/
	TCompTransform *mypos = get<TCompTransform>();
	float y, r, p;
	mypos->getYawPitchRoll(&y, &p, &r);
	float deltaYaw = mypos->getDeltaYawToAimTo(objective);
	if (fabsf(deltaYaw) <= _rotationSpeed * dt) {
		y += deltaYaw;
	}
	else {
		if (mypos->isInLeft(objective))
		{
			y += _rotationSpeed * dt;
		}
		else {
			y -= _rotationSpeed * dt;
		}
	}
	mypos->setYawPitchRoll(y, p, r);
}


void TCompDynamicObjects::StartCycle(){
	if (!_start) {
		TCompTransform *mypos = get<TCompTransform>();
		/*float y, r, p;
		mypos->getYawPitchRoll(&y, &p, &r);
		float deltaYaw = mypos->getDeltaYawToAimTo(getWaypoint());
		y = deltaYaw;
		mypos->setYawPitchRoll(y, p, r);*/
		mypos->setPosition(getWaypoint());
		
		_currentWaypoint++;
		_start = true;
	}
}
void TCompDynamicObjects::update(float dt) {
	StartCycle();
	if (_start) {
		TCompTransform *mypos = get<TCompTransform>();
		if (_currentWaypoint != _waypoints.size()) {
			//rotateTowardsVec(getWaypoint(), dt);
			VEC3 vp = mypos->getPosition();
			//Miramos si estamos lo suficientemente cerca para no pasarnos de largo
			if (VEC3::Distance(getWaypoint(), vp) < _speed * dt) {
				mypos->setPosition(getWaypoint());
				_currentWaypoint++;
				if (_currentWaypoint == _waypoints.size()) {
					_currentWaypoint = 0;
					mypos->setPosition(getWaypoint());
				}
			}
			else {
				VEC3 dir = getWaypoint() - mypos->getPosition();
				dir.Normalize();
				vp = vp + dir * _speed * dt;
				mypos->setPosition(vp);				//Move towards wpt
			}
		}
	}
	
}

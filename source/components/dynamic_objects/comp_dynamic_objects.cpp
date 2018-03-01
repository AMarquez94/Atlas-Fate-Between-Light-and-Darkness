#include "mcv_platform.h"
#include "comp_dynamic_objects.h"
#include "components\comp_transform.h"
#include "entity/entity_parser.h"
#include "render/render_utils.h"


DECL_OBJ_MANAGER("dynamic_objects", TCompDynamicObjects);


void TCompDynamicObjects::debugInMenu() {
	/*ImGui::DragFloat("X", &x, 0.1f, -20.f, 20.f);
	ImGui::DragFloat("Y", &y, 0.1f, -20.f, 20.f);
	ImGui::DragFloat("Z", &z, 0.1f, -20.f, 20.f);*/
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

void TCompDynamicObjects::StartCycle() {
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


void TCompDynamicObjects::load(const json& j, TEntityParseContext& ctx) {
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
	_target = j.value("target","");

	assert(_waypoints.size() == 2);
	_dir = _waypoints[1] - _waypoints[0];
	_dir.Normalize();

	_currentEntity = ctx.current_entity;

	Init();
}

void TCompDynamicObjects::Init() {
	_offsetShadow.x = 17;
	_offsetShadow.y = -31;
	_offsetShadow.z = 0;

	_currentWaypoint = 1;

	_playerWasInShadows = false;
	for (auto it = 0; it < _waypoints.size() - 1; it++) {
		_totalDistance += VEC3::Distance(_waypoints[it], _waypoints[it + 1]);
	}
	_numClones = _totalDistance / (_width + _offset);
	_amountMoved = _width + _offset;
	for (int it = 0; it < _numClones; it++) {
		CalculatePositions(it);
		TEntityParseContext ctx;
		parseScene("data/prefabs/capsule.prefab", ctx);
		CreateClone(ctx, it);
	}
}

void TCompDynamicObjects::CalculatePositions(int foo) {
	_pos.push_back(_waypoints[0] + _dir * _amountMoved * foo);
 }

void TCompDynamicObjects::CreateClone(TEntityParseContext ctx, int it) {
	CEntity* clone = ctx.entities_loaded[0];
	_clones.push_back(clone);
	TCompTransform *mypos = _clones[it]->get<TCompTransform>();


	mypos->setPosition(_pos[it]);
	/*if (it > 0){
		dbg("The previous position for the clone number %d is %f %f %f \n", it-1, _pos[it-1].x, _pos[it-1].y, _pos[it-1].z);
		dbg("The position for the clone number %d is %f %f %f \n", it, _pos[it].x, _pos[it].y, _pos[it].z);
	}*/
}

void TCompDynamicObjects::Move(TCompTransform* mypos, float dt) {
	//StartCycle();
	if (!_start) {
		if (_currentWaypoint != _waypoints.size()) {
			//rotateTowardsVec(getWaypoint(), dt);
			VEC3 vp = mypos->getPosition();
			//Miramos si estamos lo suficientemente cerca para no pasarnos de largo
			//Si es así
			if (VEC3::Distance(getWaypoint(), vp) < _speed * dt) {
				_currentWaypoint = 0;
				mypos->setPosition(getWaypoint());
				_currentWaypoint++;
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
bool TCompDynamicObjects::CheckProximity(float x, float y, float z) {
	//This function estimates the proximity of the target with the fake shadow
	//Hardcoded numbers to make it work
	if ((x < 0.1) && (y < 0.94) && (z < 0.45)) {
		return true;
	}
	else {
		return false;
	}
		return false;
}
bool TCompDynamicObjects::IsPlayerInShadows(VEC3 target_pos, VEC3 capsule_shadow_pos, VEC3 offset) {
	VEC3 shadow_pos(capsule_shadow_pos.x + offset.x, capsule_shadow_pos.y + offset.y, capsule_shadow_pos.z + offset.z);

	x = fabs(target_pos.x - shadow_pos.x);
	y = fabs(target_pos.y - shadow_pos.y);
	z = fabs(target_pos.z - shadow_pos.z);	


	if (CheckProximity(x, y, z)) {
		return true;
	}
	else {
		return false;
	}
}
void TCompDynamicObjects::update(float dt) {
	TCompTransform* mypos = get<TCompTransform>();
	VEC3 capsule_shadow_pos = mypos->getPosition();
	
	CEntity* target = getEntityByName(_target);
	TCompTransform* target_transform = target->get<TCompTransform>();
	VEC3 target_pos = target_transform->getPosition();

	if (!_playerWasInShadows && IsPlayerInShadows(target_pos, capsule_shadow_pos, _offsetShadow)) {
		TMsgEnteredCapsuleShadow msgEnter;
		target->sendMsg(msgEnter);
		_playerWasInShadows = true;
	}
	else if(_playerWasInShadows && !IsPlayerInShadows(target_pos, capsule_shadow_pos, _offsetShadow)) {
		TMsgExitedCapsuleShadow msgExit;
		target->sendMsg(msgExit);
		_playerWasInShadows = false;
	}
	Move(mypos, dt);

	for (int it = 0; it < _clones.size(); it++) {
		CEntity *clone_shadow = _clones[it];

		TCompTransform* mypos2 = clone_shadow->get<TCompTransform>();
		VEC3 clone_shadow_pos = mypos2->getPosition();

		if (!_playerWasInShadows && IsPlayerInShadows(target_pos, clone_shadow_pos, _offsetShadow)) {
			_currentEntity = clone_shadow;
			TMsgEnteredCapsuleShadow msgEnter;
			target->sendMsg(msgEnter);
			_playerWasInShadows = true;
		}
		else if (_playerWasInShadows && !IsPlayerInShadows(target_pos, clone_shadow_pos, _offsetShadow) && (_currentEntity == clone_shadow)) {
			TMsgExitedCapsuleShadow msgExit;
			target->sendMsg(msgExit);
			_playerWasInShadows = false;
		}
		Move(mypos2, dt);
	}	
}

#pragma once

#include "../comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"

enum FilterGroups {

	Wall = 0 << 1,
	Floor = 0 << 2,
	Player = 0 << 3,
	Enemy = 0 << 4,
	Characters = Player | Enemy,
	all = -1
};

class TCompCollider : public TCompBase {

	DECL_SIBLING_ACCESS();

public:

	struct TConfig
	{
		//todo implement
		VEC3 offset;
		VEC3 halfExtent;
		float radius;
		float height;

		int mask;
		int group;
		physx::PxGeometryType::Enum shapeType;

		bool is_dynamic; // Replace this with bitshifting
		bool is_trigger;
		bool is_character_controller;
		bool gravity;
	};

	TConfig config;
	physx::PxActor* actor;
	physx::PxController* controller;
	VEC3 delta_position;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
	void onCreate(const TMsgEntityCreated& msg);
	void onTriggerEnter(const TMsgTriggerEnter& msg);
	void onTriggerExit(const TMsgTriggerExit& msg);
};
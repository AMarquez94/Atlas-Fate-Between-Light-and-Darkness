#pragma once

#include "../comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"

class TCompCollider : public TCompBase {

	DECL_SIBLING_ACCESS();

public:

	bool isGrounded;

	struct TConfig
	{
		//todo implement
		VEC3 offset;
		VEC3 halfExtent;
		float radius;
		float height;

		unsigned int mask;
		unsigned int group;
		physx::PxGeometryType::Enum shapeType;

		bool is_dynamic; // Replace this with bitshifting
		bool is_trigger;
		bool is_character_controller;
		bool gravity;
	};

	TConfig config;
	physx::PxActor* actor;
	physx::PxController* controller;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
	void onCreate(const TMsgEntityCreated& msg);
	void onTriggerEnter(const TMsgTriggerEnter& msg);
	void onTriggerExit(const TMsgTriggerExit& msg);

	/* Aux methods, temp */
	void Resize(float new_size);
	void SetUpVector(VEC3 new_up);

	VEC3 normal_gravity = VEC3(0, -9.8f, 0);
};
#pragma once

#include "components/comp_base.h"
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

class TCompCollider: public TCompBase {

	VEC3 velocity = VEC3::Zero;

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);

	DECL_SIBLING_ACCESS();
	
public:
   
	bool isGrounded;
	bool isInside;

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

    bool is_dynamic;
    bool is_trigger;
    bool is_controller;
	bool gravity;
  };

  VEC3 lastFramePosition;

  TConfig config;
  physx::PxActor* actor;
  physx::PxController* controller;

  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  static void registerMsgs();
  void onTriggerEnter(const TMsgTriggerEnter& msg);
  void onTriggerExit(const TMsgTriggerExit& msg);

  /* Aux methods, temp */
  void Resize(float new_size);
  void SetUpVector(VEC3 new_up);
  VEC3 GetUpVector();

  VEC3 normal_gravity = VEC3(0, -9.8f, 0);
};
#pragma once

#include "comp_base.h"
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
	void onDestroy(const TMsgEntityDestroyed& msg);
	
public:
   
  struct TConfig
  {
    VEC3 halfExtent;
    
    //todo implement
    VEC3 offset;
    float radius;
	float height;
    physx::PxGeometryType::Enum shapeType;
    int group;
    int mask;
    bool is_dynamic;
    bool is_trigger;
    bool is_controller;
	bool gravity;

  };
  void onTriggerEnter(const TMsgTriggerEnter& msg);

  DECL_SIBLING_ACCESS();

  VEC3 lastFramePosition;
  TConfig config;
  physx::PxActor* actor;
  physx::PxController* controller;
  void debugInMenu();
  static void registerMsgs();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void onCreate(const TMsgEntityCreated& msg);
};
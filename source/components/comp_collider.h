#pragma once

#include "comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"

struct TMsgTriggerEnter{
  CHandle h_other_entity;
  DECL_MSG_ID();
};

struct TMsgTriggerExit {
  CHandle h_other_entity;
  DECL_MSG_ID();
};

enum FilterGroups {
  Wall = 0 << 1,
  Floor = 0 << 2,
  Player = 0 << 3,
  Enemy = 0 << 4,
  Characters = Player | Enemy,
  all = -1
};
class TCompCollider: public TCompBase {
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
    bool is_character_controller;

  };

  TConfig config;
  physx::PxActor* actor;
  physx::PxController* controller;
  void debugInMenu();
  static void registerMsgs();
  void load(const json& j, TEntityParseContext& ctx);
  void onCreate(const TMsgEntityCreated& msg);
};
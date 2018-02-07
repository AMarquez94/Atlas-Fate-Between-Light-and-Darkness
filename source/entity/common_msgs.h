#ifndef INC_COMMON_ENTITY_MSGS_
#define INC_COMMON_ENTITY_MSGS_

#include "msgs.h"
#include "geometry/transform.h"

struct TMsgEntityCreated {
  DECL_MSG_ID();
};

struct TMsgAssignBulletOwner {
  CHandle h_owner;
  DECL_MSG_ID();
};

struct TMsgDamage {
  CHandle h_sender;
  CHandle h_bullet;
  DECL_MSG_ID();
};

/*
struct TMsgTimeOut {
  DECL_MSG_ID();
};

struct TMsgSpawn {
  DECL_MSG_ID();
};

struct TMsgDefineLocalAABB {
  AABB* aabb;
  DECL_MSG_ID();
};

struct TMsgSpawnAt {
  CTransform where;
  DECL_MSG_ID();
};
*/

#endif


#pragma once

#include "components/comp_base.h"

class TCompNoClipController : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  bool isInNoClipMode = false;

  static void registerMsgs();

private:

  void onMsgNoClipToggle(const TMsgSystemNoClipToggle & msg);
  void onMsgScenePaused(const TMsgScenePaused & msg);
  VEC3 getMotionDir(const VEC3 & front, const VEC3 & left, const VEC2& movementValue);

  float speed;
  float rotationSpeed;

  VEC3 prevGravity = VEC3::Zero;
};


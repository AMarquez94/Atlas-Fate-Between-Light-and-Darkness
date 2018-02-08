#pragma once

#include "comp_base.h"
#include "camera/camera.h"
#include "entity/common_msgs.h"

class TCompCameraThirdPerson : public CCamera, public TCompBase
{
private:
	CHandle     _h_target;
	std::string _target_name;
	VEC2 _clipping_offset;

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  void mouseMovement(void);

  DECL_SIBLING_ACCESS();
};
#pragma once

#include "../comp_base.h"
#include "camera/camera.h"
#include "entity/common_msgs.h"

class TCompCameraShadowMerge : public CCamera, public TCompBase
{
private:
	CHandle     _h_target;
	std::string _target_name;

	float _speed;
	VEC2 _clamp_angle;
	VEC2 _original_euler;
	VEC2 _current_euler;
	VEC3 _clipping_offset;

	const Input::TButton& btHorizontal = EngineInput["Horizontal"];
	const Input::TButton& btVertical = EngineInput["Vertical"];
	const Input::TButton& btRHorizontal = EngineInput["MouseX"];
	const Input::TButton& btRVertical = EngineInput["MouseY"];
	const Input::TButton& btDebugPause = EngineInput["btDebugPause"];

	bool pause;
	bool active;

	void onMsgCameraActive(const TMsgCameraActivated &msg);
	void onMsgCameraDeprecated(const TMsgCameraDeprecated &msg);

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  float CameraClipping(const VEC3 & origin, const VEC3 & dir);

  DECL_SIBLING_ACCESS();

  static void registerMsgs();
};
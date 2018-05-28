#pragma once

#include "../comp_base.h"
#include "camera/camera.h"
#include "entity/common_msgs.h"

class TCompCameraThirdPerson : public CCamera, public TCompBase
{
private:
	CHandle     _h_target;
	std::string _target_name;

	float _speed;
	VEC2 _clamp_angle;
	VEC2 _current_euler;
  VEC2 _original_euler;
	VEC3 _clipping_offset;

	bool active;

	const Input::TButton& btHorizontal = EngineInput["MouseX"];
	const Input::TButton& btVertical = EngineInput["MouseY"];
	const Input::TButton& btDebugPause = EngineInput["btDebugPause"];
	void onPause(const TMsgScenePaused& msg);

	DECL_SIBLING_ACCESS();

	void onMsgCameraActive(const TMsgCameraActivated &msg);
	void onMsgCameraFullActive(const TMsgCameraFullyActivated &msg);
	void onMsgCameraDeprecated(const TMsgCameraDeprecated &msg);
	void onMsgCameraSetActive(const TMsgSetCameraActive &msg);
  void onMsgCameraReset(const TMsgCameraReset &msg);


public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  const VEC2 getCurrentEuler() { return _current_euler; }
  float CameraClipping(const VEC3 & origin, const VEC3 & dir);
  void setCurrentEuler(float euler_x = INFINITY, float euler_y = INFINITY);

  static void registerMsgs();
};
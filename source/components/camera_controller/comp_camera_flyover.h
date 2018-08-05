#pragma once

#include "components/comp_base.h"

class TCompCameraFlyover : public TCompBase
{
	DECL_SIBLING_ACCESS();

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
    void setStatus(bool status);

	static void registerMsgs();

private:
	const Input::TButton& btDebugPause = EngineInput["btDebugPause"];
	const Input::TButton& btDebugGetPos = EngineInput["btAction"];
	CHandle previousCamera;

	void onMsgActivatedMyself(const TMsgCameraActivated & msg);

	float _speed = 10.f;
	float _sensitivity = 0.005f;
	float _maxPitch = (float)M_PI_2 - 1e-4f;
};


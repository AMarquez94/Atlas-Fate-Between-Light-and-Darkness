#pragma once

#include "components/comp_base.h"

class TCompCameraFlyover : public TCompBase
{
	DECL_SIBLING_ACCESS();

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

private:
	const Input::TButton& btDebugPause = EngineInput["btDebugPause"];
	const Input::TButton& btDebugGetPos = EngineInput["btAction"];
	bool paused = false;
	CHandle previousCamera;

	float _speed = 10.f;
	float _sensitivity = 0.005f;
	float _maxPitch = (float)M_PI_2 - 1e-4f;
};


#pragma once

#include "components/comp_base.h"

class TCompCameraOrbit : public TCompBase
{
	DECL_SIBLING_ACCESS();

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

private:
	float _sensitivity = 0.005f;
	float _maxPitch = (float)M_PI_2 - 1e-4f;
	CHandle _target;
	float _distance = 5.f;
};


#pragma once

#include "components/comp_base.h"

class TCompCapsuleFinalEffects : public TCompBase
{

	DECL_SIBLING_ACCESS();
	CHandle meshRenderHandle;
	CHandle firstLight;
	CHandle secondLight;
    CHandle volumeLight;

	bool active = false;
	float time_passed = 0.0f;
	float time_for_close_lights = 0.0f;
	bool deactivate;

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
	void onMsgEntityCreated(const TMsgEntityCreated& msg);
	void onMsgEmisiveCapsuleState(const TMsgEmisiveCapsuleState& msg);
};

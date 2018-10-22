#pragma once

#include "components/comp_base.h"

class TCompLightDestroyFinalScene : public TCompBase
{

	DECL_SIBLING_ACCESS();

	bool active = false;
	float time_to_destroy = 0.0f;
	float current_time = 0.0f;
public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
	void onMsgEntityCreated(const TMsgEntityCreated& msg);
	void onMsgEmisiveCapsuleState(const TMsgEmisiveCapsuleState& msg);
};

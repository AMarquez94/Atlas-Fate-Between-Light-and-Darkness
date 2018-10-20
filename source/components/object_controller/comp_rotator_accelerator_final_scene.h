#pragma once

#include "components/comp_base.h"

class TCompRotatorAcceleratorFinalScene : public TCompBase
{

	DECL_SIBLING_ACCESS();
	CHandle rotator_handle;
public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
	void onMsgEntityCreated(const TMsgEntityCreated& msg);
	void onMsgStartAcceleration(const TMsgRotatorAccelerate& msg);
};

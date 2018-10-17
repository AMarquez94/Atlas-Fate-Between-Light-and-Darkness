#pragma once

#include "components/comp_base.h"

class TCompCapsuleFinalEffects : public TCompBase
{

	DECL_SIBLING_ACCESS();
	CHandle meshRenderHandle;
	CHandle firstLightHandle;
	CHandle secondLightHandle;

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
	void onMsgEntityCreated(const TMsgEntityCreated& msg);
};

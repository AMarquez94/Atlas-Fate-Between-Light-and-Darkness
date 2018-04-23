#pragma once

#include "../comp_base.h"
#include "../comp_camera.h"

class CTexture;
class CRenderToTexture;

class TCompEmissionController : public TCompCamera {

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);

	DECL_SIBLING_ACCESS();
public:

	void debugInMenu();
	void renderDebug();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
};
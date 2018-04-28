#pragma once

#include "../comp_base.h"
#include "../comp_camera.h"

class CTexture;
class CRenderToTexture;

class TCompEmissionController : public TCompCamera {

	VEC4 current_color;
	VEC4 desired_color;
	float blendInTime = 1.f;

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);

	DECL_SIBLING_ACCESS();
public:

	void debugInMenu();
	void renderDebug();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	void blendColors(VEC4 new_desired_color, float blendTime);

	static void registerMsgs();
};
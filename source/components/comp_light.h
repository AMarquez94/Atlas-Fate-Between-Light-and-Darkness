#pragma once

#include "comp_base.h"
#include "geometry/transform.h"
#include "comp_camera.h"

class CTexture;

class TCompLight : public TCompCamera {

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);

	DECL_SIBLING_ACCESS();
public:

	std::string type;

	float intensity = 1.0f;
	VEC4 color = VEC4(1,1,1,1);
	const CTexture* projector = nullptr;

	/* Light flags */
	bool cast_shadows;

	/* spotlight parameters */
	float range;
	float angle;

	void activate();
	void debugInMenu();
	void renderDebug();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
};
#pragma once

#include "comp_base.h"
#include "geometry/transform.h"

class TCompLight : public TCompBase {

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);

	DECL_SIBLING_ACCESS();
public:

	std::string type;

	VEC3 color;
	float intensity;

	/* Light flags */
	bool cast_shadows;

	/* spotlight parameters */
	float range;
	float angle;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void BuildTrigger(void);

	static void registerMsgs();
};
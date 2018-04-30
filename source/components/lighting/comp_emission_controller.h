#pragma once

#include "../comp_base.h"
#include "../comp_camera.h"

class CTexture;
class CMaterial;
class CRenderToTexture;
class TCompLightSpot;

class TCompEmissionController : public TCompCamera {

	std::vector<CMaterial*> _temp_materials;
	std::vector<TCompLightSpot*> _temp_lights;

	VEC4 _original_color;
	VEC4 _current_color;
	VEC4 _desired_color;

	float _elapsed_time = 0.f;
	float _blend_in_time = 0.1f;

	void onSceneCreated(const TMsgSceneCreated& msg);

	DECL_SIBLING_ACCESS();

public:

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	//void turnOn(void);
	//void turnOff(void);
	void blend(VEC4 new_desired_color, float blendTime);

	static void registerMsgs();
};
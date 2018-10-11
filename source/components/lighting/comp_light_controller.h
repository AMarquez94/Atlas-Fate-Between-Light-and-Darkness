#pragma once

#include "../comp_base.h"
#include "../comp_camera.h"

class CTexture;
class CMaterial;
class CRenderToTexture;
class TCompLightSpot;
class TCompLightPoint;

class TCompLightController : public TCompBase {

    CHandle _parent;
    TCompLightSpot * _spot_light;
	TCompLightPoint * _point_light;
    TCompRender * _object_render;

    float _emissive_intensity;
    std::string _emissive_target;

    float _intensity;
    float _intensity_flow;
    float _intensity_flow_speed;
    std::vector<float> _scripted_times;

    float _radius;
    float _radius_flow;
    float _radius_flow_speed;

    bool _has_flicker;
    VEC2 _flicker_time;

    float _off_time;
    float _random_time;
    float _flicker_elapsed_time = 0.f;

	float _elapsed_time = 0.f;

	void onSceneCreated(const TMsgSceneCreated& msg);
    void onGroupCreated(const TMsgEntitiesGroupCreated& msg);

    void updateMovement(float dt);
    void updateIntensity(float dt);
    void updateFlicker(float dt);

	DECL_SIBLING_ACCESS();

public:

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
};
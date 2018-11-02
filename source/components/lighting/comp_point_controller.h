#pragma once

#include "../comp_base.h"
#include "../comp_camera.h"

class CTexture;
class CMaterial;
class CRenderToTexture;
class TCompLightSpot;
class TCompLightPoint;

class TCompPointController : public TCompBase {

    CHandle _parent;
    TCompRender * _mesh_render;
    TCompRender * _object_render;
	TCompLightPoint * _point_light;
    TCompParticles * _object_particles;
    TCompCollider * _object_collider;

    float _emissive_intensity;
    std::string _emissive_target;
    std::string _light_target;
    std::string _mesh_target;

    float _intensity;
    float _intensity_flow;
    float _intensity_flow_speed;
    float _random_time = 0;

    int _current_burst;
    std::vector<VEC3> _bursts;

    float _radius;
    float _radius_flow;
    float _radius_flow_speed;

    float _elapsed_time = 0.f;
    float _flicker_elapsed_time = 0.f;
    bool _flicker_status = false;

    SoundEvent flicker_sound;

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
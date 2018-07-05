#pragma once

#include "../comp_base.h"
#include "../comp_camera.h"

class CTexture;
class CRenderToTexture;

struct TInstanceLight {
    MAT44 world;
    VEC4 light_pos;
    VEC4 light_dir;
    VEC4 light_values;
    MAT44 project_offset;
};

class TCompLightSpot : public TCompCamera {

	VEC4			color = VEC4(1, 1, 1, 1);
	float			intensity = 1.0f;

    //CRenderMesh * spotcone;
    int  num_samples = 80;

	// Shadows params
    bool              cull_enabled = false;      // Dynamic
    bool              volume_enabled = false;      // Static
	bool              shadows_enabled = false;    // Dynamic
	bool              casts_shadows = false;      // Static

	int               shadows_resolution = 256;
	float             shadows_step = 1.f;
    CRenderToTexture* shadows_rt = nullptr;

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);

	DECL_SIBLING_ACCESS();
public:

    static CRenderMeshInstanced* volume_instance;
	const CTexture* projector = nullptr;

	/* spotlight parameters */
	float range;
	float angle;
	float inner_cut;
	float outer_cut;
	bool is_moving;

	void debugInMenu();
	void renderDebug();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	void activate();
	void generateShadowMap();
    void generateVolume();
    void cullFrame();

	MAT44 getWorld();
	void createAABB();

	void setColor(const VEC4 & new_color);

	static void registerMsgs();
};
#pragma once

#include "../comp_base.h"

class TCompLightPoint : public TCompBase {

	// Light params
	VEC4            color = VEC4(1, 1, 1, 1);
	
    bool            cull_enabled = false;      // Dynamic

    void onCreate(const TMsgEntityCreated& msg);
    void onGroupCreated(const TMsgEntitiesGroupCreated& msg);
    void onSetVisible(const TMsgSetVisible& msg);

    DECL_SIBLING_ACCESS();

public:
	float           intensity = 1.0f;
    float inner_cut = 1.0f;
    float outer_cut = 1.0f;

    bool isEnabled;
	const CTexture* projector = nullptr;

	void debugInMenu();
	void renderDebug();
	void load(const json& j, TEntityParseContext& ctx);

	void activate();
    void cullFrame();
    bool isCulled() const;

	MAT44 getWorld();
    VEC4 getColor() const;
    void setColor(VEC4 new_color);

    void setRadius(float new_radius);
    float getRadius();

    void setIntensity(float new_int);
    float getIntensity();

    bool interactsWithPlayer = true;

    static void registerMsgs();
};
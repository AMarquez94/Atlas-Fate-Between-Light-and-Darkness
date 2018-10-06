#pragma once

#include "../comp_base.h"
#include "../comp_camera.h"

class TCompFadeController : public TCompBase {

    bool _is_drownable;
    bool _is_active;
	VEC4 _fade_color;
	VEC4 _original_fade_color;
    std::string _material;

	float _elapsed_time = 0.f;
	float _fade_time;
	float _original_fade_time;
    float _fall_speed;

    int _invert_fade;

	DECL_SIBLING_ACCESS();

public:

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	void launch(const TMsgFadeBody& msg);
    void destroy(void);

    void setFadeTime(float new_time) { _fade_time = new_time; };
    float getFadeTime() { return _fade_time; };

    void setFallSpeed(float new_speed) { _fall_speed = new_speed; };
    float getFallSpeed() { return _fall_speed; };

    static void registerMsgs();
};
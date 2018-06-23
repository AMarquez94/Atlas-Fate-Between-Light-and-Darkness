#include "mcv_platform.h"
#include "comp_fade_controller.h"
#include "render/render_objects.h"
#include "render/render_utils.h"
#include "components/comp_transform.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/comp_name.h"
#include "components/lighting/comp_light_spot.h"
#include "render/texture/material.h"
#include "components/physics/comp_rigidbody.h"
#include "physics/physics_collider.h"
#include "ctes.h" 

DECL_OBJ_MANAGER("fade_controller", TCompFadeController);

void TCompFadeController::debugInMenu() {

}

void TCompFadeController::load(const json& j, TEntityParseContext& ctx) {

	if (j.count("fade_color"))
        _fade_color = loadVEC4(j["fade_color"]);

    _fade_time = j.value("fade_time", 6.0f);
    _fall_speed = j.value("fall_speed", 0.1f);
    _material = j.value("material", "data/materials/characters/mtl_player_main_shade.material");
    _is_active = false;
}

/* Update the values during the given time */
void TCompFadeController::update(float dt) {

    if (cb_player.player_fade > 1 && _is_active) {
        _is_active = false;
        destroy();
    }

    if (_is_active) {
        TCompTransform * transform = get<TCompTransform>();
        transform->setPosition(transform->getPosition() - VEC3(0, _fall_speed * dt, 0));
        cb_player.player_fade += (dt / _fade_time); // Tweak here due to noise intensity
        //cb_player.updateGPU();
    }
}

/* Used to blend between two colors at a given time */
void TCompFadeController::launch() {
    
    if (_is_active) return;

    _is_active = true;
    _elapsed_time = 0.f;
    cb_player.player_fade = 0;

    TCompRender * c_my_render = get<TCompRender>();
    TCompRigidbody * c_my_rigid = get<TCompRigidbody>();
    c_my_rigid->is_enabled = false;

    assert(c_my_render);
    c_my_render->setMaterial(_material);
}

void TCompFadeController::destroy() {

    CHandle(this).getOwner().destroy();
}
#include "mcv_platform.h"
#include "comp_fade_controller.h"
#include "render/render_objects.h"
#include "render/render_utils.h"
#include "components/comp_transform.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/lighting/comp_light_spot.h"
#include "render/texture/material.h"
#include "ctes.h" 

DECL_OBJ_MANAGER("fade_controller", TCompFadeController);

void TCompFadeController::debugInMenu() {

}

void TCompFadeController::load(const json& j, TEntityParseContext& ctx) {

	if (j.count("fade_color"))
        _fade_color = loadVEC4(j["fade_color"]);

    _fade_time = j.value("_fade_time", 2.0f);
    _is_active = false;
}

/* Update the values during the given time */
void TCompFadeController::update(float dt) {

    if (_elapsed_time > _fade_time && _is_active)
        destroy();

	cb_player.player_fade += dt / _fade_time;
    cb_player.updateGPU();
}

/* Used to blend between two colors at a given time */
void TCompFadeController::launch() {
    
    _is_active = true;
    _elapsed_time = 0.f;
    cb_player.player_fade = 0;

    TCompRender * c_my_render = get<TCompRender>();
    assert(c_my_render);
    c_my_render->setMaterial("data/materials/characters/mtl_player_main_shade.material");
}

void TCompFadeController::destroy() {

    CHandle(this).getOwner().destroy();
}
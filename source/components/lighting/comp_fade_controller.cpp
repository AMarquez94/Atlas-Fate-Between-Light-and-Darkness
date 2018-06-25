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

    _is_drownable = j.value("destroy", true);
    _fade_time = j.value("fade_time", 6.0f);
    _fall_speed = j.value("fall_speed", 0.1f);
    _material = j.value("material", "data/materials/characters/mtl_player_main_shade.material");

    _invert_fade = 1;
    _is_active = false;
}

void TCompFadeController::registerMsgs() {

    DECL_MSG(TCompFadeController, TMsgFadeBody, launch);
}

/* Update the values during the given time */
void TCompFadeController::update(float dt) {

    if (_is_active) {

        // Notify it has finished
        if (cb_object.self_fade_value > 1 || cb_object.self_fade_value < 0) {

            _is_active = false;
            if (_is_drownable) destroy();
        }

        // Moves the body downwards
        if (_is_drownable) {

            TCompTransform * transform = get<TCompTransform>();
            transform->setPosition(transform->getPosition() - VEC3(0, _fall_speed * dt, 0));
        }

        cb_object.self_fade_value += (dt / _fade_time) * _invert_fade; // Tweak here due to noise intensity
    }
}

/* Used to blend between two colors at a given time */
void TCompFadeController::launch(const TMsgFadeBody& msg) {
    
    _is_active = true;
    _elapsed_time = 0.f;
    _invert_fade = msg.is_exit ? -1 : 1;

    cb_object.self_fade_value = msg.is_exit ? 1 : 0;
    cb_object.updateGPU();

    if (_is_drownable) {

        TCompRigidbody * c_my_rigid = get<TCompRigidbody>();
        assert(c_my_rigid);
        c_my_rigid->is_enabled = false;
    }

    TCompRender * c_my_render = get<TCompRender>();
    assert(c_my_render);
    c_my_render->setMaterial(_material);
}

void TCompFadeController::destroy() {

    CHandle(this).getOwner().destroy();
}
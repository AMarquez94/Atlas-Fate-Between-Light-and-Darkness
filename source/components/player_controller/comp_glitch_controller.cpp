#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_glitch_controller.h"
#include "components/comp_transform.h"
#include "components/comp_render.h"
#include "entity/common_msgs.h"
#include "utils/utils.h"
#include "components/comp_tags.h"
#include "components/comp_name.h"
#include "render/texture/material.h"
#include "render/render_objects.h"
#include "render/render_utils.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("glitch_controller", TCompGlitchController);

void TCompGlitchController::debugInMenu() {

}

void TCompGlitchController::load(const json& j, TEntityParseContext& ctx) {

    fade_multiplier = 1;
    fade_time = j.value("fade_time", 4);

    cb_postfx.postfx_scan_amount = 0; // Disable the scan.
    cb_postfx.updateGPU();
}

void TCompGlitchController::update(float dt) {

    // Move this from here on refactor
    cb_postfx.postfx_scan_amount = Clamp(cb_postfx.postfx_scan_amount, 0.f, 1.f);
    cb_outline.updateGPU();
}

void TCompGlitchController::onGlitchDeploy(const TMsgGlitchController & msg) {

}

void TCompGlitchController::registerMsgs() {

    DECL_MSG(TCompGlitchController, TMsgGlitchController, onGlitchDeploy);
}

#include "mcv_platform.h"
#include "comp_emission_controller.h"
#include "../comp_transform.h"
#include "render/render_objects.h"    // cb_light
#include "render/texture/texture.h"
#include "render/texture/render_to_texture.h" 
#include "render/render_manager.h" 
#include "render/render_utils.h"
#include "render/gpu_trace.h"
#include "ctes.h"                     // texture slots

DECL_OBJ_MANAGER("emission_controller", TCompEmissionController);

void TCompEmissionController::debugInMenu() {

}

void TCompEmissionController::load(const json& j, TEntityParseContext& ctx) {


}

void TCompEmissionController::update(float dt) {


}

void TCompEmissionController::registerMsgs() {

	DECL_MSG(TCompEmissionController, TMsgEntityCreated, onCreate);
	DECL_MSG(TCompEmissionController, TMsgEntityDestroyed, onDestroy);
}

void TCompEmissionController::onCreate(const TMsgEntityCreated& msg) {

}

void TCompEmissionController::onDestroy(const TMsgEntityDestroyed & msg) {

}

void TCompEmissionController::blendColors(VEC4 new_desired_color, float blendTime) {

	blendInTime = blendTime;
	desired_color = new_desired_color;
}
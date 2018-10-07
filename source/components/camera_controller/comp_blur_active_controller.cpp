#include "mcv_platform.h"
#include "components/camera_controller/comp_blur_active_controller.h"
#include "components/postfx/comp_render_blur.h"
#include "components/postfx/comp_render_focus.h"

DECL_OBJ_MANAGER("blur_active_controller", TCompBlurActiveController);

void TCompBlurActiveController::debugInMenu() {

}

void TCompBlurActiveController::load(const json& j, TEntityParseContext& ctx) {
	
}

void TCompBlurActiveController::update(float dt) {

	CEntity *e = CHandle(this).getOwner();
	TCompRenderFocus *comp_render_focus = e->get<TCompRenderFocus>();
	if (comp_render_focus != nullptr && comp_render_focus->enabled == true) {
		TCompRenderBlur *comp_render_blur = e->get<TCompRenderBlur>();
		comp_render_focus->enabled = false;
		if (comp_render_blur != nullptr) {
			comp_render_blur->enabled = false;
		}
	}
}
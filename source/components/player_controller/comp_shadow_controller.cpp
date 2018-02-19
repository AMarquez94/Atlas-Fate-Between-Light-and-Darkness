#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_shadow_controller.h"
#include "../comp_transform.h"
#include "../comp_render.h"
#include "entity/common_msgs.h"
#include "utils/utils.h"
#include "render/mesh/mesh_loader.h"
#include "render/render_objects.h"
#include "components/comp_camera.h"
#include "components/physics/comp_collider.h"

DECL_OBJ_MANAGER("shadow_controller", TCompShadowController);

void TCompShadowController::debugInMenu() {

}

void TCompShadowController::load(const json& j, TEntityParseContext& ctx) {

}

void TCompShadowController::Init() {


}

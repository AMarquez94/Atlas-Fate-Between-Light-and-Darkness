#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_sonar_controller.h"
#include "components/comp_transform.h"
#include "components/comp_render.h"
#include "entity/common_msgs.h"
#include "utils/utils.h"
#include "render/mesh/mesh_loader.h"
#include "render/render_objects.h"
#include "components/comp_camera.h"
#include "components/physics/comp_collider.h"
#include "components/comp_tags.h"
#include "components/lighting/comp_light_dir.h"
#include "components/lighting/comp_light_spot.h"
#include "components/lighting/comp_light_point.h"
#include "components/comp_name.h"

DECL_OBJ_MANAGER("sonar_controller", TCompSonarController);

void TCompSonarController::debugInMenu() {
}

void TCompSonarController::load(const json& j, TEntityParseContext& ctx) {

}

void TCompSonarController::update(float dt) {

    //CTimer timer = new timer();
    //timer.
}


void TCompSonarController::registerMsgs() {

    //DECL_MSG(TCompSonarController, TMsgSceneCreated, onSceneCreated);
    //DECL_MSG(TCompSonarController, TMsgPlayerIlluminated, onPlayerExposed);
}

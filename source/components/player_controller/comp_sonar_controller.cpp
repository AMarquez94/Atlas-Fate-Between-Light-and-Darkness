#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_sonar_controller.h"
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

DECL_OBJ_MANAGER("sonar_controller", TCompSonarController);

void TCompSonarController::debugInMenu() {
}

void TCompSonarController::load(const json& j, TEntityParseContext& ctx) {

    target_tag = j.value("tags", "");
    total_time = j.value("alive_time", 0);
    cooldown_time = j.value("cooldown_time", 0);

    alpha_value = 0;
    cb_outline.outline_alpha = 0; // Move this from here
    cb_outline.updateGPU();
}

void TCompSonarController::update(float dt) {

    // Move this from here on refactor
    cb_outline.linear_time += dt;
    cb_outline.outline_alpha = cb_outline.linear_time >= total_time ? cb_outline.outline_alpha  - 0.5f * dt: alpha_value;
    cb_outline.updateGPU();
}

void TCompSonarController::onSonarActive(const TMsgSonarActive & msg) {

    CEntity* e = CHandle(this).getOwner();
    target_handles = CTagsManager::get().getAllEntitiesByTag(getID(target_tag.c_str()));
    cb_outline.linear_time = 0;
    alpha_value = msg.value;

    TMsgSetFSMVariable sonar;
    sonar.variant.setName("sonar");
    sonar.variant.setBool(false);
    e->sendMsg(sonar);

    /* Enable this in case we want to hold the alpha value by material
    for (auto p : target_handles) {
        CEntity * c_entity = p;
        TCompRender * c_render = c_entity->get<TCompRender>();
        for (auto m : c_render->meshes) {
            for (auto mtl : m.materials) {
                (const_cast<CMaterial*>(mtl))->setCBMaterial(msg.value);
            }
        }
    }*/
}

const bool TCompSonarController::canDeploySonar() {

    CEntity* e = CHandle(this).getOwner();
    TCompTempPlayerController * c_my_player = e->get<TCompTempPlayerController>();

    if (!c_my_player->isDead() && !c_my_player->isMerged 
        && c_my_player->isGrounded && !c_my_player->isInhibited 
        && cb_outline.linear_time > cooldown_time && !Engine.getGameManager().menuVisible)
        return true;

    return false;
}

void TCompSonarController::registerMsgs() {

    DECL_MSG(TCompSonarController, TMsgSonarActive, onSonarActive);
}

#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_circular_controller.h"
#include "../comp_transform.h"

DECL_OBJ_MANAGER("circular_controller", TCompCircularController);

void TCompCircularController::debugInMenu() {

    ImGui::DragFloat2("Radius", &radius.x, 0.1f, 0.f, 20.f);
    ImGui::DragFloat("Speed", &speed, 0.1f, 0.f, 20.f);
    ImGui::DragFloat("Oscilation Speed", &oscilation_speed, 0.1f, 0.f, 100.f);

    ImGui::Text("Yaw %f", rad2deg(curr_yaw));
    if (h_target.isValid()) {
        if (ImGui::TreeNode("Target")) {
            h_target.debugInMenu();
            ImGui::TreePop();
        }
    }
}

void TCompCircularController::onCreate(const TMsgEntityCreated& msg) {
    TCompTransform *c_my_transform = get<TCompTransform>();
    curr_height = c_my_transform->getPosition().y;
}

void TCompCircularController::onNewTarget(const TMsgCircularControllerTarget & msg)
{
    h_target = msg.new_target;
    CEntity* e_test = h_target;
}

void TCompCircularController::registerMsgs() {

    DECL_MSG(TCompCircularController, TMsgEntityCreated, onCreate);
    DECL_MSG(TCompCircularController, TMsgCircularControllerTarget, onNewTarget);
}

void TCompCircularController::load(const json& j, TEntityParseContext& ctx) {


    float t_radius = j.value("radius", 1.0f);
    if (j.count("vradius"))
        radius = loadVEC2(j["vradius"]);
    else
        radius = VEC2(t_radius, t_radius);

    speed = j.value("speed", 1.0f);
    vertical_speed = j.value("vertical_speed", 0.f);
    oscilation_speed = j.value("oscilation_speed", 0.f);
    target_name = j.value("target", "");
  
    h_target = ctx.findEntityByName(target_name);
    oscilation_range = (radius.y - radius.x) * .5f;
}

void TCompCircularController::update(float dt) {

    if (!h_target.isValid())
        return;

    curr_yaw += speed * dt;
    curr_height += vertical_speed * dt;
    total_time += dt;

    CEntity* e_target = h_target;
    assert(e_target);

    TCompTransform *c_target = e_target->get<TCompTransform>();
    assert(c_target);

    TCompTransform *c_my_transform = get<TCompTransform>();
    float t_radius = radius.x + (oscilation_range + sin(oscilation_speed * total_time) * oscilation_range);

    float my_y = c_my_transform->getPosition().y;
    VEC3 my_new_pos = c_target->getPosition() + getVectorFromYaw(curr_yaw) * t_radius;
    my_new_pos.y = my_y + curr_height;

    assert(c_my_transform);
    c_my_transform->lookAt(my_new_pos, c_target->getPosition());
}


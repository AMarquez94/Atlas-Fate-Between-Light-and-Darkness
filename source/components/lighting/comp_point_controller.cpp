#include "mcv_platform.h"
#include "comp_point_controller.h"
#include "components/comp_transform.h"
#include "components/comp_render.h"
#include "components/lighting/comp_light_spot.h"
#include "components/lighting/comp_light_point.h"
#include "components/comp_group.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("point_controller", TCompPointController);

// Refactor this with point light and spot light superclass to avoid conditional testing.
void TCompPointController::debugInMenu() {

    ImGui::DragFloat("Intensity Flow: ", &_intensity_flow, 0.01, 0, 100);
    ImGui::DragFloat("Intensity Flow Speed: ", &_intensity_flow_speed, 0.01, 0, 100);
    ImGui::DragFloat("Radius Flow: ", &_radius_flow, 0.01, 0, 100);
    ImGui::DragFloat("Radius Flow Speed: ", &_radius_flow_speed, 0.01, 0, 100);
    ImGui::DragFloat("Flicker Off Time: ", &_off_time, 0.01, 0, 100);
    ImGui::DragFloat2("Flicker Time Interval: ", &_flicker_time.x, 0.01, 0, 100);
}

void TCompPointController::load(const json& j, TEntityParseContext& ctx) {

    _intensity_flow = j.value("intensity_flow", 0.0f);
    _intensity_flow_speed = j.value("intensity_flow_speed", 0.0f);

    _radius_flow = j.value("radius_flow", 0.0f);
    _radius_flow_speed = j.value("radius_flow_speed", 0.0f);
    _emissive_intensity = j.value("emissive_intensity", 0.0f);
    _emissive_target = j.value("emissive_target", "");
    _flicker_time = j.count("flicker_time") ? loadVEC2(j["flicker_time"]) : VEC2::Zero;
    _has_flicker = j.value("has_flicker", false);
    _off_time = j.value("off_time", 0.0f);
    _parent = ctx.entities_loaded.back();

    // Pick a random time between these two values
    _random_time = urand(_flicker_time.x, _flicker_time.y);
}

/* Update the values during the given time */
void TCompPointController::update(float dt) {

    _elapsed_time += dt;
    updateIntensity(dt);
    updateMovement(dt);
    updateFlicker(dt);
}

void TCompPointController::registerMsgs() {

	DECL_MSG(TCompPointController, TMsgSceneCreated, onSceneCreated);
    DECL_MSG(TCompPointController, TMsgEntitiesGroupCreated, onGroupCreated);
}

/* Used to retrieve the total materials from our render component */
void TCompPointController::onSceneCreated(const TMsgSceneCreated& msg) {

    CEntity * owner = CHandle(this).getOwner();
    CEntity * parent = _parent;
    if (_emissive_target != "") {
        TCompGroup* cGroup = parent->get<TCompGroup>();
        CEntity* eCone = cGroup->getHandleByName(_emissive_target);
        if (eCone) {
            _object_render = eCone->get<TCompRender>();
            if (_object_render) _emissive_intensity = _object_render->self_intensity;
        }
    }

    _point_light = owner->get<TCompLightPoint>();

    if (_point_light) {
        _radius = _point_light->getRadius();
        _intensity = _point_light->getIntensity();
    }

    if (!_object_render) {
        _object_render = get<TCompRender>();
        if (_object_render) _emissive_intensity = _object_render->self_intensity;
    }
}

void TCompPointController::onGroupCreated(const TMsgEntitiesGroupCreated & msg)
{
    CEntity * owner = CHandle(this).getOwner();
    /*if (_emissive_target != "") {
        TCompGroup* cGroup = get<TCompGroup>();
        CEntity* eCone = cGroup->getHandleByName(_emissive_target);
        if (eCone) _object_render = eCone->get<TCompRender>();
    }*/
}

void TCompPointController::updateMovement(float dt)
{
    float new_radius = _radius + _radius_flow * sin(_elapsed_time * _radius_flow_speed);

    if (_point_light) _point_light->setRadius(new_radius);
}

void TCompPointController::updateIntensity(float dt)
{
    float variable_flow = _intensity_flow * sin(_elapsed_time * _intensity_flow_speed);
    float new_intensity = _intensity + variable_flow;

    if (_point_light) _point_light->setIntensity(new_intensity);

    //if (_object_render) {
    //    _object_render->self_intensity = _emissive_intensity + variable_flow;
    //}
}

void TCompPointController::updateFlicker(float dt)
{
    _flicker_elapsed_time += dt;
    if (_has_flicker && _flicker_elapsed_time > _random_time) {

        if (_point_light) _point_light->isEnabled = false;
        if (_object_render)  _object_render->self_intensity = 0;

        if (_flicker_elapsed_time > (_random_time + _off_time)) {
            if (_point_light) {
                _point_light->isEnabled = true;
                _point_light->setIntensity(_intensity);
            }

            if (_object_render)  _object_render->self_intensity = _emissive_intensity;

            _elapsed_time = 0;
            _flicker_elapsed_time = 0;
            _random_time = urand(_flicker_time.x, _flicker_time.y);
        }
    }
}

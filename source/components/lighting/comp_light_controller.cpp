#include "mcv_platform.h"
#include "comp_emission_controller.h"
#include "render/render_objects.h"
#include "render/render_utils.h"
#include "components/comp_transform.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/lighting/comp_light_spot.h"
#include "render/texture/material.h"
#include "ctes.h" 

DECL_OBJ_MANAGER("emission_controller", TCompEmissionController);

void TCompEmissionController::debugInMenu() {
    ImGui::ColorEdit4("Current color: ", &_current_color.x);
    ImGui::ColorEdit4("Desired color: ", &_desired_color.x);
}

void TCompEmissionController::load(const json& j, TEntityParseContext& ctx) {

	_current_color = VEC4(1, 1, 1, 1);

	if (j.count("initial"))
		_current_color = loadVEC4(j["initial"]);

    _intensity = j.value("intensity", 1.0f);
	_desired_color = _current_color;
	_original_color = _current_color;
}

/* Update the values during the given time */
void TCompEmissionController::update(float dt) {

	if (_elapsed_time < 1) {

		_elapsed_time += dt / _blend_in_time;
		_current_color = VEC4::Lerp(_original_color, _desired_color, _elapsed_time);

        TCompRender * self_render = get<TCompRender>();
        if (self_render) {
          self_render->self_color = _current_color;
        }

        // Deprecated method, update this if color per mesh is needed.
		//for (auto p : _temp_materials)
		//	p->changeEmissionColor(_current_color);

		//for (auto p : _temp_lights)
		//	p->setColor(_current_color);
	}
}

void TCompEmissionController::registerMsgs() {

	DECL_MSG(TCompEmissionController, TMsgSceneCreated, onSceneCreated);
}

/* Used to retrieve the total materials from our render component */
void TCompEmissionController::onSceneCreated(const TMsgSceneCreated& msg) {

	// First, retrieve all possible spotlights from the owner  -- deactivated because we are not changing the lights
	//TCompGroup * self_group = get<TCompGroup>();
	//assert(self_group);

	//for (auto p : self_group->handles) {
	//	CEntity * ent = (CEntity*)p;
	//	TCompLightSpot * spotlight = ent->get<TCompLightSpot>();
	//	if (spotlight)
	//		_temp_lights.push_back(spotlight);
	//}

  TCompRender * self_render = get<TCompRender>();
  assert(self_render);
  self_render->self_color = _current_color;
  self_render->self_intensity = _intensity;
	//for (auto p : self_render->meshes)
	//	for (auto m : p.materials)
	//		_temp_materials.push_back(const_cast<CMaterial*>(m));
}

/* Used to blend between two colors at a given time */
void TCompEmissionController::blend(VEC4 new_desired_color, float blendTime) {

	_elapsed_time = 0.0f;
	_blend_in_time = blendTime;

	_original_color = _current_color;
	_desired_color = new_desired_color;
}
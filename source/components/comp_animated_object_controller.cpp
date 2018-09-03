#include "mcv_platform.h"
#include "comp_animated_object_controller.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("animated_object_controller", TCompAnimatedObjController);

void TCompAnimatedObjController::registerMsgs() {

}

void TCompAnimatedObjController::debugInMenu() {

}

void TCompAnimatedObjController::load(const json& j, TEntityParseContext& ctx) {
	/*
	if (j.count("animations")) {
		auto& j_animations = j["animations"];
		for (auto it = j_animations.begin(); it != j_animations.end(); ++it) {
			RigidAnims::CController controller;
			controller.track_name = it.value().value("track", "");
			if (controller.track_name.empty()) {
				CEntity* e = ctx.current_entity;
				assert(e);
				controller.track_name = e->getName();
			}
			assert(it.value().count("src") > 0);
			controller.anims = Resources.get(it.value()["src"])->as<RigidAnims::CRigidAnimResource>();
			controller.track_index = controller.anims->findTrackIndexByName(controller.track_name);
			assert(controller.track_index != RigidAnims::CController::invalid_track_index);
			current_time = 0;
			speed_factor = it.value().value("speed_factor", 1.0f);
			loops = it.value().value("loops", true);
			controllers.push_back(controller);

		}
	}
	parent_position = VEC3(0, 0, -20);
	*/
}

void TCompAnimatedObjController::update(float dt) {
	//Crec que no s'utilitzara, pero nose lol jajaja omg que nup
}


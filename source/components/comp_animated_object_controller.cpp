#include "mcv_platform.h"
#include "comp_animated_object_controller.h"
#include "components/comp_transform.h"
#include "comp_rigid_anim.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("animated_object_controller", TCompAnimatedObjController);

void TCompAnimatedObjController::registerMsgs() {
	DECL_MSG(TCompAnimatedObjController, TMsgEntitiesGroupCreated, onGroupCreated);
}

void TCompAnimatedObjController::onGroupCreated(const TMsgEntitiesGroupCreated &msg) {

	for (int i = 0; i < object_names.size(); i++) {
		CEntity* e = getEntityByName(object_names[i]);

		if (e != nullptr) {
			TCompRigidAnim* comp_rigid = e->get<TCompRigidAnim>();
			object_handles.push_back(e);

			if (comp_rigid != nullptr) {

				for (int j = 0; j < animationInfos.size(); j++) {
					comp_rigid->registerAnimation(animationInfos[j].animationName, animationInfos[j].track_name, animationInfos[j].source, animationInfos[j].speedFactor, animationInfos[j].loop,j);
					name_to_id_animations[animationInfos[j].animationName] = j;
				}		
			}
		}		
	}
}

void TCompAnimatedObjController::debugInMenu() {

}

void TCompAnimatedObjController::load(const json& j, TEntityParseContext& ctx) {

	if (j.count("reference_names")) {
		auto& j_references = j["reference_names"];
		for (auto it = j_references.begin(); it != j_references.end(); ++it) {

			std::string name = it.value().value("name", "");
			object_names.push_back(name);

		}
	}

	if (j.count("animations")) {
		auto& j_animations = j["animations"];
		for (auto it = j_animations.begin(); it != j_animations.end(); ++it) {
			AnimationInfo ainfo;

			ainfo.animationName = it.value().value("name", "");
			ainfo.track_name = it.value().value("track", "");
			assert(it.value().count("src") > 0);
			ainfo.source = it.value().value("src","");
			ainfo.speedFactor = it.value().value("speed_factor", 1.0f);
			ainfo.loop = it.value().value("loops", true);

			animationInfos.push_back(ainfo);
		}
	}

}

void TCompAnimatedObjController::update(float dt) {
	//Crec que no s'utilitzara, pero nose lol jajaja omg que nup
}

void TCompAnimatedObjController::playAnimation(std::string anim_name) {
	int id = name_to_id_animations[anim_name];
	for (int i = 0; i < object_handles.size(); i++) {
		CEntity *e = object_handles[i];
		TCompRigidAnim *rigid_anim = e->get<TCompRigidAnim>();
		rigid_anim->playAnimation(id);
	}
}


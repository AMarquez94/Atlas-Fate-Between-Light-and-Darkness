#include "mcv_platform.h"
#include "comp_animated_object_controller.h"
#include "components/comp_transform.h"
#include "comp_rigid_anim.h"
#include "components/comp_fsm.h"
#include "comp_group.h"

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
					CEntity *ownEntity = CHandle(this).getOwner();
					TCompTransform *comp_transform = ownEntity->get<TCompTransform>();
					comp_rigid->registerParentTransform(comp_transform->getPosition(), comp_transform->getRotation());
					name_to_id_animations[animationInfos[j].animationName] = j;
				}		
			}
		}		
	}

	if (getNames) {

		TCompGroup * comp_group = get<TCompGroup>();
		VHandles handle_vector = comp_group->handles;
		for (int i = 0; i < handle_vector.size() ; i++) {

			object_handles.push_back(handle_vector[i]);
			CEntity *e = handle_vector[i];
			TCompRigidAnim* comp_rigid = e->get<TCompRigidAnim>();
			if (comp_rigid != nullptr) {

				for (int j = 0; j < animationInfos.size(); j++) {
					comp_rigid->registerAnimation(animationInfos[j].animationName, animationInfos[j].track_name, animationInfos[j].source, animationInfos[j].speedFactor, animationInfos[j].loop, j);
					CEntity *ownEntity = CHandle(this).getOwner();
					TCompTransform *comp_transform = ownEntity->get<TCompTransform>();
					comp_rigid->registerParentTransform(comp_transform->getPosition(), comp_transform->getRotation());
					name_to_id_animations[animationInfos[j].animationName] = j;
				}
			}
		}

	}

	if (starting_animation.size() > 0) playAnimation(starting_animation);
}

void TCompAnimatedObjController::debugInMenu() {
	for (int i = 0; i < animationInfos.size(); i++) {
		
		if (ImGui::SmallButton(animationInfos[i].animationName.c_str())) {
			playAnimation(animationInfos[i].animationName);
		}
		
	}
	
}

void TCompAnimatedObjController::load(const json& j, TEntityParseContext& ctx) {

	if (j.count("reference_names")) {
		auto& j_references = j["reference_names"];
		if (j_references[0].count("get_names_auto")) {
			getNames = true;
		}
		else {
			for (auto it = j_references.begin(); it != j_references.end(); ++it) {

				std::string name = it.value().value("name", "");
				object_names.push_back(name);

			}
		}	
	}

	if (j.count("animations")) {
		auto& j_animations = j["animations"];
		for (auto it = j_animations.begin(); it != j_animations.end(); ++it) {
			AnimationInfo ainfo;

			assert(it.value().count("name") > 0);
			ainfo.animationName = it.value().value("name", "");
			ainfo.track_name = it.value().value("track", "");
			assert(it.value().count("src") > 0);
			ainfo.source = it.value().value("src","");
			ainfo.speedFactor = it.value().value("speed_factor", 1.0f);
			ainfo.loop = it.value().value("loops", true);
			if(it.value().value("on_start", false)) starting_animation = ainfo.animationName;
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


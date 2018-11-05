#include "mcv_platform.h"
#include "comp_capsule_final_effects.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/lighting/comp_light_point.h"

DECL_OBJ_MANAGER("capsule_final_effect", TCompCapsuleFinalEffects);

void TCompCapsuleFinalEffects::debugInMenu() {

}

void TCompCapsuleFinalEffects::load(const json& j, TEntityParseContext& ctx) {

}

void TCompCapsuleFinalEffects::update(float dt) {

	if (active) {
		if (time_passed >= time_for_close_lights) {
			firstLight.destroy();
			secondLight.destroy();
            volumeLight.destroy();
			active = false;
		}
		time_passed += dt;
	}
}

void TCompCapsuleFinalEffects::registerMsgs() {
	DECL_MSG(TCompCapsuleFinalEffects, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompCapsuleFinalEffects, TMsgEmisiveCapsuleState, onMsgEmisiveCapsuleState);
	
}

void TCompCapsuleFinalEffects::onMsgEntityCreated(const TMsgEntityCreated& msg) {

	CEntity* ent = CHandle(this).getOwner();
	TCompGroup *group = ent->get<TCompGroup>();
	CEntity* meshCapsula = group->handles[2];
	CEntity* meshLight1 = group->handles[0];
	CEntity* meshLight2 = group->handles[1];
    CEntity* meshVolume = group->handles[3];
	meshRenderHandle = meshCapsula->get<TCompRender>();
	firstLight = meshLight1->get<TCompLightPoint>();
	secondLight = meshLight2->get<TCompLightPoint>();
    volumeLight = meshVolume->get<TCompLightPoint>();;
}

void TCompCapsuleFinalEffects::onMsgEmisiveCapsuleState(const TMsgEmisiveCapsuleState& msg) {
	active = msg.enable;
	deactivate = msg.enable;
	float time_f = 0.0f;
	if (msg.random) {
		int rand_time = 250 + ((std::rand()) % (1000 - 250 + 1));
		time_f = (float)rand_time / 100.0f;
		time_for_close_lights = time_f;
	}

	if (deactivate) {
		TCompRender* renderMesh = meshRenderHandle;
		EngineLerp.lerpElement(&renderMesh->self_intensity, 0.0, msg.time_to_lerp, time_f);
	}
	else {
		TCompRender* renderMesh = meshRenderHandle;
		EngineLerp.lerpElement(&renderMesh->self_intensity, 5.0, msg.time_to_lerp, time_f);
	}
	
}
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
	//TCompRender* renderMesh = meshRenderHandle;
	//renderMesh->self_intensity -= dt * 3;
	
}

void TCompCapsuleFinalEffects::registerMsgs() {
	DECL_MSG(TCompCapsuleFinalEffects, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompCapsuleFinalEffects, TMsgEmisiveCapsuleState, onMsgEmisiveCapsuleState);
	
}

void TCompCapsuleFinalEffects::onMsgEntityCreated(const TMsgEntityCreated& msg) {

	CEntity* ent = CHandle(this).getOwner();
	TCompGroup *group = ent->get<TCompGroup>();
	CEntity* meshCapsula = group->handles[2];
	meshRenderHandle = meshCapsula->get<TCompRender>();

}

void TCompCapsuleFinalEffects::onMsgEmisiveCapsuleState(const TMsgEmisiveCapsuleState& msg) {
	active = true;
	deactivate = msg.enable;

	TCompRender* renderMesh = meshRenderHandle;
	//rand();
	EngineLerp.lerpElement(&renderMesh->self_intensity, 1.0, 4, 0);
}
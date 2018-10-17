#include "mcv_platform.h"
#include "comp_capsule_final_effects.h"
#include "components/comp_render.h"
#include "components/comp_group.h"

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
}

void TCompCapsuleFinalEffects::onMsgEntityCreated(const TMsgEntityCreated& msg) {

	CEntity* ent = CHandle(this).getOwner();
	TCompGroup *group = ent->get<TCompGroup>();
	CEntity* light1 = group->handles[0];
	CEntity* light2 = group->handles[1];
	CEntity* meshCapsula = group->handles[2];
	secondLightHandle = meshCapsula->get<TCompRender>();
	firstLightHandle = meshCapsula->get<TCompRender>();
	meshRenderHandle = meshCapsula->get<TCompRender>();
}
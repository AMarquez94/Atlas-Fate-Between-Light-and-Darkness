#include "mcv_platform.h"
#include "comp_capsule_shadow.h"
#include "components/comp_transform.h"
#include "components/comp_render.h"


DECL_OBJ_MANAGER("capsule_shadow_controller", TCompCapsuleShadow);


void TCompCapsuleShadow::debugInMenu() {
}

void TCompCapsuleShadow::load(const json& j, TEntityParseContext& ctx) {
	maxZ = j.value("max_z", 80.f);
}

void TCompCapsuleShadow::update(float dt) {
	TCompTransform* mypos = get<TCompTransform>();
	TCompRender * tRender = get < TCompRender>();
	if (mypos->getPosition().z > maxZ) {
		if (tRender->visible) {
			tRender->visible = false;
		}
	}
	else {
		if (!tRender->visible) {
			tRender->visible = true;
		}
	}
}

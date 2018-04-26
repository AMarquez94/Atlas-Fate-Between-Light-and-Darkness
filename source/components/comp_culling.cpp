#include "mcv_platform.h"
#include "comp_culling.h"
#include "comp_aabb.h"
#include "comp_camera.h"
//#include "comp_msgs.h"

DECL_OBJ_MANAGER("culling", TCompCulling);

void TCompCulling::debugInMenu() {
	auto hm = getObjectManager<TCompAbsAABB>();
	ImGui::Text("%d / %d AABB's visible", bits.count(), hm->size());
}

// Si algun plano tiene la caja en la parte negativa
// completamente, el aabb no esta dentro del set de planos
bool TCompCulling::VPlanes::isVisible(const AABB* aabb) const {
	auto it = begin();
	while (it != end()) {
		if (it->isCulled(aabb))
			return false;
		++it;
	}
	return true;
}

void TCompCulling::updateFromMatrix(MAT44 view_proj)
{
	// Construir el set de planos usando la view_proj
	planes.fromViewProjection(view_proj);

	// Start from zero
	bits.reset();

	// Traverse all aabb's defined in the game
	// and test them
  // Use the AbsAABB index to access the bitset
	auto hm = getObjectManager<TCompAbsAABB>();
	hm->forEachWithExternalIdx([this](const TCompAbsAABB* aabb, uint32_t external_idx) {
		if (planes.isVisible(aabb))
			bits.set(external_idx);
	});
}

void TCompCulling::update(float dt) {
	//PROFILE_FUNCTION("Updating culling");

	// Conseguimos acceso al comp_camera de un sibling component
	TCompCamera* c_camera = get<TCompCamera>();
	if (!c_camera)
		return;

	MAT44 view_proj = c_camera->getViewProjection();
	//e_owner->sendMsg(TMsgGetCullingViewProj{ &view_proj });

	updateFromMatrix(view_proj);
}

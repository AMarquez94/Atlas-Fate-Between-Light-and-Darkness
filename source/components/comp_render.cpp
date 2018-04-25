#include "mcv_platform.h"
#include "comp_render.h"
#include "comp_transform.h"
#include "skeleton/comp_skeleton.h"

DECL_OBJ_MANAGER("render", TCompRender);

#include "render/render_objects.h"
#include "render/texture/texture.h"
#include "render/texture/material.h"
#include "render/render_utils.h"
#include "render/render_manager.h"
#include "entity/entity_parser.h"


TCompRender::~TCompRender() {
	// Delete all references of me in the render manager
	CRenderManager::get().delRenderKeys(CHandle(this));
}

// --------------------------------------------
void TCompRender::onDefineLocalAABB(const TMsgDefineLocalAABB& msg) {
	AABB::CreateMerged(*msg.aabb, *msg.aabb, aabb);
}

void TCompRender::registerMsgs() {
	DECL_MSG(TCompRender, TMsgDefineLocalAABB, onDefineLocalAABB);
}

void TCompRender::debugInMenu() {
	ImGui::ColorEdit4("Color", &color.x);
	//for (auto &m : materials) {
	//	if (m)
	//		((CMaterial*)m)->debugInMenu();
	//}
}

void TCompRender::renderDebug() {

	//activateRSConfig(RSCFG_WIREFRAME);
	//TCompTransform * transform = get<TCompTransform>();
	//assert(transform);
	//
	////If we have an skeleton, make sure the required bones are actived and updated
	//TCompSkeleton* skel = get<TCompSkeleton>();
	//if (skel) {
	//	skel->updateCtesBones();
	//	skel->cb_bones.activate();
	//}

	//for (auto& mwm : meshes)
	//	renderMesh(mwm.mesh, transform->asMatrix(), color);
	//activateRSConfig(RSCFG_DEFAULT);
}

void TCompRender::loadMesh(const json& j, TEntityParseContext& ctx) {

	CHandle(this).setOwner(ctx.current_entity);
	CMeshWithMaterials mwm;
	std::string name_mesh = j.value("mesh", "axis.mesh");
	mwm.mesh = Resources.get(name_mesh)->as<CRenderMesh>();

	if (j.count("materials")) {
		auto& j_mats = j["materials"];
		assert(j_mats.is_array());
		for (size_t i = 0; i < j_mats.size(); ++i) {
			// Allow to define a null and not render that material idx of the mesh
			const CMaterial* material = nullptr;
			if (j_mats[i].is_string()) {
				std::string name_material = j_mats[i];
				material = Resources.get(name_material)->as<CMaterial>();
			}
			mwm.materials.push_back(material);
		}
		assert(mwm.materials.size() <= mwm.mesh->getSubGroups().size());
	}
	else {
		const CMaterial* material = Resources.get("data/materials/solid.material")->as<CMaterial>();
		mwm.materials.push_back(material);
	}

	mwm.enabled = j.value("enabled", true);

	// If there is a color in the json, read it
	if (j.count("color"))
		color = loadVEC4(j["color"]);

	AABB::CreateMerged(aabb, aabb, mwm.mesh->getAABB());

	meshes.push_back(mwm);
}

void TCompRender::load(const json& j, TEntityParseContext& ctx) {

	// Reset the AABB
	aabb.Center = VEC3(0, 0, 0);
	aabb.Extents = VEC3(0, 0, 0);

	// We expect an array of things to render: mesh + materials, mesh + materials, ..
	if (j.is_array()) {
		for (size_t i = 0; i < j.size(); ++i)
			loadMesh(j[i], ctx);
	}
	else {
		// We accept not receiving an array of mesh inside the comp_render, for handle files
		loadMesh(j, ctx);
	}

	refreshMeshesInRenderManager();

	if (j.count("COLOR"))
		color = loadVEC4(j["color"]);
}

void TCompRender::refreshMeshesInRenderManager() {
	CHandle h_me = CHandle(this);
	CRenderManager::get().delRenderKeys(h_me);

	// The house and the trees..
	for (auto& mwm : meshes) {

		// Do not register disabled meshes
		if (!mwm.enabled)
			continue;

		// All materials of the house...
		uint32_t idx = 0;
		for (auto& m : mwm.materials) {
			// Supporting null materials to discard submeshes
			if (m) {
				CRenderManager::get().addRenderKey(
					h_me,
					mwm.mesh,
					m,
					idx
				);
			}
			++idx;
		}
	}
}
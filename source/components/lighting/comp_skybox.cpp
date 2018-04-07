#include "mcv_platform.h"
#include "comp_skybox.h"
#include "../comp_transform.h"
#include "render/render_objects.h"    // cb_light
#include "render/texture/texture.h"
#include "ctes.h"                     // texture slots
#include "render/texture/render_to_texture.h" 
#include "render/render_manager.h" 
#include "render/render_utils.h"
#include "render/gpu_trace.h"
#include "render/texture/material.h"

DECL_OBJ_MANAGER("skybox", TCompSkybox);


void TCompSkybox::load(const json& j, TEntityParseContext& ctx) {

	// Load the mesh and the default material
	// Loading this way to avoid having the skybox as an entity
	//mesh = Resources.get("data/meshes/skybox_basic.mesh")->as<CRenderMesh>();
	//material = Resources.get("data/materials/render_skybox.material")->as<CMaterial>();
	//const CTexture * tex = Resources.get("data/materials/default_skybox.dds")->as<CTexture>();

	// Load texture provided by user and set it to the material
	//material->textures[0] = tex;
}


void TCompSkybox::update(float dt) {

	// We move the translation from here
	// Since there was a problem with the module render order.
}

void TCompSkybox::activate() {

	TCompTransform * trans = get<TCompTransform>();
	trans->setPosition(cb_camera.camera_pos);
}

void TCompSkybox::renderDebug() {

	//auto prev_tech = CRenderTechnique::current;
	//MAT44 model_world = MAT44::CreateTranslation(cb_camera.camera_pos);
	//setWorldTransform(model_world, VEC4(1, 1, 1, 1));

	////activateOMConfig(OMConfig::OMCFG_LESS_EQUAL);
	//material->activate();
	//mesh->activateAndRender();
	//prev_tech->activate();
	//activateOMConfig(OMConfig::OMCFG_DEFAULT);
}



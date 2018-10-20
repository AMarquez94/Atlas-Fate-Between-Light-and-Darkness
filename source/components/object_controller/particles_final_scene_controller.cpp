#include "mcv_platform.h"
#include "particles_final_scene_controller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/comp_particles.h"

DECL_OBJ_MANAGER("particles_final_scene_controller", TCompParticlesFinalSceneController);

void TCompParticlesFinalSceneController::debugInMenu() {

}

void TCompParticlesFinalSceneController::load(const json& j, TEntityParseContext& ctx) {

}

void TCompParticlesFinalSceneController::update(float dt) {
}

void TCompParticlesFinalSceneController::registerMsgs() {
	DECL_MSG(TCompParticlesFinalSceneController, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompParticlesFinalSceneController, TMsgRotatorAccelerate, onMsgWheelRotating);

}

void TCompParticlesFinalSceneController::onMsgEntityCreated(const TMsgEntityCreated& msg) {

}

void TCompParticlesFinalSceneController::onMsgWheelRotating(const TMsgRotatorAccelerate& msg) {

	if (msg.speed != 0.0f) return;

	CEntity *ent = CHandle(this).getOwner();
	TCompParticles *comp_particles = ent->get<TCompParticles>();
	comp_particles->setSystemFading(7.0);
}
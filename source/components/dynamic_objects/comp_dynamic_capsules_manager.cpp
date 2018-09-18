#include "mcv_platform.h"
#include "comp_dynamic_capsules_manager.h"
#include "comp_dynamic_capsule.h"
#include "components/comp_transform.h"
#include "components/comp_group.h"
#include "components/comp_name.h"
#include "components/physics/comp_collider.h"
#include "entity/entity_parser.h"
#include "components/physics/comp_collider.h"
#include "physics/physics_collider.h"
#include "components/comp_audio.h"

DECL_OBJ_MANAGER("dynamic_capsules_manager", TCompDynamicCapsulesManager);

void TCompDynamicCapsulesManager::debugInMenu() {

}

void TCompDynamicCapsulesManager::load(const json & j, TEntityParseContext & ctx)
{

	start_point = loadVEC3(j["start_point"]);
	end_point = loadVEC3(j["end_point"]);
	numberOfCapsules = j.value("number", 0);
	speed = j.value("speed", 0.f);
    capsule_offset = loadVEC3(j["capsule_offset"]);

	if (numberOfCapsules > 0) {

		VEC3 offset = (end_point - start_point) / (float)(numberOfCapsules);

		/* Creates component group whose "head" is this entity */
		CHandle h_group = getObjectManager<TCompGroup>()->createHandle();
		CEntity * e = ctx.current_entity;
		e->set(h_group.getType(), h_group);
		TCompGroup *cGroup = h_group;

		VEC3 newPos;

		for (int i = 0; i < numberOfCapsules; i++) {

			/* Create capsule */
			TEntityParseContext ctxCapsules;
			parseScene("data/prefabs/dynamic_capsule.prefab", ctxCapsules);
			CEntity *eCapsule = ctxCapsules.entities_loaded[0];

			/* Set name */
			TCompName *capsuleName = eCapsule->get<TCompName>();
			capsuleName->setName(("Capsule_" + std::to_string(i)).c_str());

			/* Set pos */
            TCompCollider *capsuleCollider = eCapsule->get<TCompCollider>();
            VEC3 temp_offset = PXVEC3_TO_VEC3(capsuleCollider->config->center);
			TCompTransform *capsuleTransform = eCapsule->get<TCompTransform>();
			newPos = start_point + (float)i * offset;
			capsuleTransform->setPosition(newPos + temp_offset);
            float delta_yaw = capsuleTransform->getDeltaYawToAimTo(end_point);
            float yaw, pitch;
            capsuleTransform->getYawPitchRoll(&yaw, &pitch);
            capsuleTransform->setYawPitchRoll(yaw + delta_yaw, pitch);
			cGroup->add(eCapsule);

			/* Set dynamic capsule component */
			TCompDynamicCapsule *cDynamicCapsule = eCapsule->get<TCompDynamicCapsule>();
			cDynamicCapsule->setStartPoint(start_point + temp_offset);
			cDynamicCapsule->setEndPoint(end_point + temp_offset);
			cDynamicCapsule->setSpeed(speed);

            TCompAudio *cAudio = eCapsule->get<TCompAudio>();
            cAudio->playEvent("event:/Sounds/Objects/Capsules/Caspule");
		}
	}
}

void TCompDynamicCapsulesManager::update(float dt) {

}

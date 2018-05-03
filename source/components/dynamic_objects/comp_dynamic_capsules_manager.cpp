#include "mcv_platform.h"
#include "comp_dynamic_capsules_manager.h"
#include "comp_dynamic_capsule.h"
#include "components/comp_transform.h"
#include "components/comp_group.h"
#include "components/comp_name.h"
#include "components/physics/comp_collider.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("dynamic_capsules_manager", TCompDynamicCapsulesManager);

void TCompDynamicCapsulesManager::debugInMenu() {

}

void TCompDynamicCapsulesManager::load(const json & j, TEntityParseContext & ctx)
{

	start_point = loadVEC3(j["start_point"]);
	end_point = loadVEC3(j["end_point"]);
	numberOfCapsules = j.value("number", 0);
	speed = j.value("speed", 0.f);

	if (numberOfCapsules > 0) {

		VEC3 offset = (end_point - start_point) / (float)(numberOfCapsules + 1);

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
			TCompTransform *capsuleTransform = eCapsule->get<TCompTransform>();
			newPos = start_point + (float)i * offset;
			capsuleTransform->setPosition(newPos);
			cGroup->add(eCapsule);

			/* Set dynamic capsule component */
			TCompDynamicCapsule *cDynamicCapsule = eCapsule->get<TCompDynamicCapsule>();
			cDynamicCapsule->setStartPoint(start_point);
			cDynamicCapsule->setEndPoint(end_point);
			cDynamicCapsule->setSpeed(speed);

			/* Set rigidbody pos */
			TCompCollider *capsuleCollider = eCapsule->get<TCompCollider>();
			QUAT quat = capsuleTransform->getRotation();
			capsuleCollider->setGlobalPose(newPos, quat, false);
		}
	}
}

void TCompDynamicCapsulesManager::update(float dt) {

}

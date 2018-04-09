#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_shadow_controller.h"
#include "components/comp_transform.h"
#include "components/comp_render.h"
#include "entity/common_msgs.h"
#include "utils/utils.h"
#include "render/mesh/mesh_loader.h"
#include "render/render_objects.h"
#include "components/comp_camera.h"
#include "components/physics/comp_collider.h"
#include "components/comp_tags.h"
#include "components/comp_light.h"
#include "../comp_name.h"

DECL_OBJ_MANAGER("shadow_controller", TCompShadowController);

void TCompShadowController::debugInMenu() {

}

void TCompShadowController::load(const json& j, TEntityParseContext& ctx) {

	Init();
}

void TCompShadowController::update(float dt) {

	TCompTransform * c_my_transform = get<TCompTransform>();
	VEC3 new_pos = c_my_transform->getPosition() + 0.1f * c_my_transform->getUp();
	is_shadow = IsPointInShadows(new_pos);
	//is_shadow == true ? dbg("i'm in shadow\n") : dbg("i'm in light\n");
 }

void TCompShadowController::Init() {

	is_shadow = false;
	// Retrieve all scene lights
}

void TCompShadowController::onSceneCreated(const TMsgSceneCreated& msg) {

	auto& light_handles = CTagsManager::get().getAllEntitiesByTag(getID("light"));

	for (auto h : light_handles) {
		CEntity* current_light = h;
		TCompLight * c_light = current_light->get<TCompLight>();
		if (c_light == NULL) continue;
		if (c_light->type == "directional") // by now we will only retrieve directional lights
		{
			static_lights.push_back(h);
		}
		else if (c_light->type == "spotlight" || c_light->type == "pointlight") // by now we will only retrieve directional lights
		{
			TCompCollider * c_collider = current_light->get<TCompCollider>();
			if (c_collider != NULL)
				dynamic_lights.push_back(h);
		}
	}

	physx::PxFilterData pxFilterData;
	//pxFilterData.word0 = EnginePhysics.FilterGroup::Scenario;
	pxFilterData.word1 = FilterGroup::Fence;
	shadowDetectionFilter.data = pxFilterData;
}

void TCompShadowController::onEnteredCapsuleShadow(const TMsgEnteredCapsuleShadow& msg) {

	capsule_shadow = true;
}

void TCompShadowController::onExitedCapsuleShadow(const TMsgExitedCapsuleShadow& msg) {

	capsule_shadow = false;
}

void TCompShadowController::registerMsgs() {

	DECL_MSG(TCompShadowController, TMsgSceneCreated, onSceneCreated);
	DECL_MSG(TCompShadowController, TMsgEnteredCapsuleShadow, onEnteredCapsuleShadow);
	DECL_MSG(TCompShadowController, TMsgExitedCapsuleShadow, onExitedCapsuleShadow);
}

// We can also use this public method from outside this class.
bool TCompShadowController::IsPointInShadows(const VEC3 & point)
{
	if (capsule_shadow) {
		return true;
	}

	physx::PxRaycastHit hit;
	for (unsigned int i = 0; i < static_lights.size(); i++) {
		CEntity * c_entity = static_lights[i];
		TCompTransform * c_trans = c_entity->get<TCompTransform>();

		float distance = VEC3::Distance(c_trans->getPosition(), point);
		if (!EnginePhysics.Raycast(point, c_trans->getUp(), distance, hit, physx::PxQueryFlag::eSTATIC, shadowDetectionFilter))
			return false;
	}

	for (unsigned int i = 0; i < dynamic_lights.size(); i++)
	{
		CEntity * c_entity = dynamic_lights[i];
		TCompCollider * c_collider = c_entity->get<TCompCollider>();
		TCompTransform * c_transform = c_entity->get<TCompTransform>();
		if (c_collider->player_inside)
		{
			VEC3 dir = point - c_transform->getPosition();
			dir.Normalize();
			float distance = VEC3::Distance(c_transform->getPosition(), point);
			if (!EnginePhysics.Raycast(c_transform->getPosition(), dir, distance - 0.2f, hit, physx::PxQueryFlag::eSTATIC, shadowDetectionFilter))
				return false;
		}
	}

	return true;
}

/* Method used to generate local points, we will apply transform later on */
void TCompShadowController::GenerateSurroundingPoints(const VEC3 & point)
{
	float radius = 0.35f;
	float nradius = (2 * (radius - 0.02f)); // Hardcoded for colliding purposes
	for (int x = 0; x < test_levels; x++)
	{
		for (int y = 0; y < test_amount; y++)
		{
			float lRadius = nradius / (x + 1);
			float t = y / (float)test_amount;
			float h = .5f * lRadius * cosf(t * 2.f * (float)M_PI) + point.x;
			float v = .5f * lRadius * sinf(t * 2.f * (float)M_PI) + point.z;
		}
	}
}

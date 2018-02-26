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
#include "components/player_controller/comp_player_controller.h"
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

	TCompPlayerController * pc = get<TCompPlayerController>();
	if (!pc->checkPaused()) {
		// Do it for n points, every update. 
		// Change this later to a fixed update function, avoid lot of raycast checks...

		TCompTransform * c_my_transform = get<TCompTransform>();
		VEC3 new_pos = c_my_transform->getPosition() + 0.1f * c_my_transform->getUp();
		is_shadow = IsPointInShadows(new_pos);

		//is_shadow == true ? dbg("i'm in shadow\n") : dbg("i'm in light\n");
	}
 }

void TCompShadowController::Init() {

	is_shadow = false;
	// Retrieve all scene lights
}

void TCompShadowController::onSceneCreated(const TMsgSceneCreated& msg) {

	auto& handles = CTagsManager::get().getAllEntitiesByTag(getID("light"));

	for (auto h : handles) {
		CEntity* current_light = h;
		TCompLight * c_light = current_light->get<TCompLight>();
		if (c_light->type == "directional") // by now we will only retrieve directional lights
		{
			static_lights.push_back(c_light);
		}
	}
}

void TCompShadowController::registerMsgs() {

	DECL_MSG(TCompShadowController, TMsgSceneCreated, onSceneCreated);
}


// We can also use this public method from outside this class.
bool TCompShadowController::IsPointInShadows(const VEC3 & point)
{
	//VEC3 light_dir = VEC3(-0.7663f, -0.384137f, -0.514998f); // Hardcoded for testing purposes only!!!
	// We need a safe system to retrieve the light direction and origin spot.
	// Also we need to distinguish between light types.
	// Different light tests must be made.

	CModulePhysics::RaycastHit hit;
	for (unsigned int x = 0; x < static_lights.size(); x++) {
		CEntity * c_entity = CHandle(static_lights[x]).getOwner();
		TCompTransform * c_trans = c_entity->get<TCompTransform>();
		VEC3 front = c_trans->getFront();

		float distance = VEC3::Distance(c_trans->getPosition(), point);
		if (!EnginePhysics.Raycast(point, c_trans->getFront(), distance, hit, EnginePhysics.eSTATIC))
			return false;

		if (EnginePhysics.Raycast(point, c_trans->getFront(), distance, hit, EnginePhysics.eSTATIC))
		{
			CEntity * ent = CHandle(hit.transform).getOwner();
			TCompName * text = ent->get<TCompName>();
			dbg("entity name: %s", text->getName());
		}
	}

	// Get all entites tagged as dynamic light, and test them.
	// Use their transform to do the check.

	return true;
}

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
			float h = .5f * lRadius * cosf(t * 2 * M_PI) + point.x;
			float v = .5f * lRadius * sinf(t * 2 * M_PI) + point.z;
		}
	}
}

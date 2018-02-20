#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_shadow_controller.h"
#include "../comp_transform.h"
#include "../comp_render.h"
#include "entity/common_msgs.h"
#include "utils/utils.h"
#include "render/mesh/mesh_loader.h"
#include "render/render_objects.h"
#include "components/comp_camera.h"
#include "components/physics/comp_collider.h"

DECL_OBJ_MANAGER("shadow_controller", TCompShadowController);

void TCompShadowController::debugInMenu() {

}

void TCompShadowController::load(const json& j, TEntityParseContext& ctx) {

	static_points.emplace_back(VEC3(56.436,48.786,31.666));
	Init();
}

void TCompShadowController::update(float dt) {

	// Do it for n points, every update. 
	// Change this later to a fixed update function, avoid lot of raycast checks...

	TCompTransform * c_my_transform = get<TCompTransform>();
	VEC3 new_pos = c_my_transform->getPosition() + 0.1f * c_my_transform->getUp();
	is_shadow = IsPointInShadows(new_pos);

	//is_shadow == true ? dbg("i'm in shadow\n") : dbg("i'm in light\n");
 }

void TCompShadowController::Init() {

	is_shadow = false;
	// Retrieve all scene lights
}

// We can also use this public method from outside this class.
bool TCompShadowController::IsPointInShadows(const VEC3 & point)
{
	VEC3 light_dir = VEC3(-0.7663, -0.384137, -0.514998); // Hardcoded for testing purposes only!!!
	// We need a safe system to retrieve the light direction and origin spot.
	// Also we need to distinguish between light types.
	// Different light tests must be made.

	CModulePhysics::RaycastHit hit;
	for (unsigned int x = 0; x < static_points.size(); x++) {

		float distance = VEC3::Distance(static_points[x], point);
		if (!CEngine::get().getPhysics().Raycast(point, -light_dir, distance, hit))
			return false;
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
			float h = .5f * lRadius * cos(t * 2 * M_PI) + point.x;
			float v = .5f * lRadius * sin(t * 2 * M_PI) + point.z;
		}
	}
}

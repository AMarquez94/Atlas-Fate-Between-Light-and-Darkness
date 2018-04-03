#include "mcv_platform.h"
#include "module_map_test.h"
#include "camera/camera.h"
#include "render/render_objects.h"
#include "entity/entity.h"
#include "modules/module_entities.h"
#include "resources/resources_manager.h"
#include "render/texture/texture.h"
#include "render/texture/material.h"
#include "handle/handle.h"
#include "components/comp_name.h"
#include "components/comp_transform.h"
#include "components/comp_camera.h"
#include "entity/entity_parser.h"
#include "components/comp_tags.h"
#include "render/render_manager.h"
#include "input/devices/mouse.h"
#include "input/enums.h"

bool CModuleMapTest::start()
{
	json jboot = loadJson("data/boot.json");
	
	// Auto load some scenes
	std::vector< std::string > scenes_to_auto_load = jboot["light_scenes_test"];
	for (auto& scene_name : scenes_to_auto_load) {
		dbg("Autoloading scene %s\n", scene_name.c_str());
		TEntityParseContext ctx;
		parseScene(scene_name, ctx);
	}

	//camera.lookAt(VEC3(12.0f, 8.0f, 8.0f), VEC3::Zero, VEC3::UnitY);
	//camera.setPerspective(60.0f * 180.f / (float)M_PI, 0.1f, 1000.f);

	auto om = getObjectManager<CEntity>();
	om->forEach([](CEntity* e) {
		TMsgSceneCreated msg;
		CHandle h_e(e);
		h_e.sendMsg(msg);
	});

	Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
	mouse->setLockMouse(true);
	ShowCursor(false);

	return true;
}

bool CModuleMapTest::stop()
{
	Engine.getEntities().destroyAllEntities();
	Engine.getCameras().deleteAllCameras();
	Engine.getIA().clearSharedBoards();

	return true;
}

void CModuleMapTest::update(float delta)
{
	static VEC3 world_pos;
	ImGui::DragFloat3("Pos", &world_pos.x, 0.025f, -50.f, 50.f);
	VEC2 mouse = EngineInput.mouse()._position;
}

void CModuleMapTest::render()
{
	// Render the grid
	cb_object.obj_world = MAT44::Identity;
	cb_object.obj_color = VEC4(1, 1, 1, 1);
	cb_object.updateGPU();

	auto solid = Resources.get("data/materials/solid.material")->as<CMaterial>();
	solid->activate();

	//auto grid = Resources.get("grid.mesh")->as<CRenderMesh>();
	//grid->activateAndRender();
	auto axis = Resources.get("axis.mesh")->as<CRenderMesh>();
	axis->activateAndRender();

}
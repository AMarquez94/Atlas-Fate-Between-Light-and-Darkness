#include "mcv_platform.h"
#include "module_map_intro.h"
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

CCamera camera;

bool CModuleMapIntro::start()
{
	json jboot = loadJson("data/boot.json");
	
	// Auto load some scenes
	std::vector< std::string > scenes_to_auto_load = jboot["boot_scenes"];
	for (auto& scene_name : scenes_to_auto_load) {
		dbg("Autoloading scene %s\n", scene_name.c_str());
		TEntityParseContext ctx;
		parseScene(scene_name, ctx);
	}
	{
		TEntityParseContext ctx;
		parseScene("data/scenes/bt_test.scene", ctx);
	}

	camera.lookAt(VEC3(12.0f, 8.0f, 8.0f), VEC3::Zero, VEC3::UnitY);
	camera.setPerspective(60.0f * 180.f / (float)M_PI, 0.1f, 1000.f);

	// -------------------------------------------
	if (!cb_camera.create(CB_CAMERA))
		return false;
	// -------------------------------------------
	if (!cb_object.create(CB_OBJECT))
		return false;

	if (!cb_light.create(CB_LIGHT))
		return false;

	cb_object.activate();
	cb_camera.activate();

	CHandle h_camera = getEntityByName("TPCamera");
	if (h_camera.isValid())
		Engine.getCameras().setDefaultCamera(h_camera);

	h_camera = getEntityByName("main_camera");
	if (h_camera.isValid())
		Engine.getCameras().setOutputCamera(h_camera);

	auto om = getObjectManager<CEntity>();
	om->forEach([](CEntity* e) {
		TMsgSceneCreated msg;
		CHandle h_e(e);
		h_e.sendMsg(msg);
	});

	Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
	mouse->setLockMouse();
	ShowCursor(false);

	return true;
}

bool CModuleMapIntro::stop()
{
	/* delete all entities in scene */
	Engine.getEntities().destroyAllEntities();
	Engine.getCameras().deleteAllCameras();
	Engine.getIA().clearSharedBoards();

	cb_camera.destroy();
	cb_object.destroy();
	cb_light.destroy();

	return true;
}

void CModuleMapIntro::update(float delta)
{
	static VEC3 world_pos;
	ImGui::DragFloat3("Pos", &world_pos.x, 0.025f, -50.f, 50.f);

	VEC2 mouse = EngineInput.mouse()._position;
	if (h_e_camera.isValid()) {
		CEntity* e_camera = h_e_camera;
		TCompCamera* c_camera = e_camera->get< TCompCamera >();

		VEC3 screen_coords;
		bool inside = c_camera->getScreenCoordsOfWorldCoord(world_pos, &screen_coords);
		ImGui::Text("Inside: %s  Coords: %1.2f, %1.2f  Z:%f", inside ? "YES" : "NO ", screen_coords.x, screen_coords.y, screen_coords.z);
		ImGui::Text("Mouse at %1.2f, %1.2f", mouse.x, mouse.y);
	}
}

void CModuleMapIntro::render()
{
	// Find the entity with name 'the_camera'
	h_e_camera = getEntityByName("main_camera");
	if (h_e_camera.isValid()) {
		CEntity* e_camera = h_e_camera;
		TCompCamera* c_camera = e_camera->get< TCompCamera >();
		assert(c_camera);
		activateCamera(*c_camera);
		CRenderManager::get().setEntityCamera(h_e_camera);
	}
	else {
		activateCamera(camera);
	}

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
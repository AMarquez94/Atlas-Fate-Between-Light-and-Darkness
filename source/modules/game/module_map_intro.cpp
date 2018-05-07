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
#include "input/enums.h"
#include "modules/system/module_sound.h"

bool CModuleMapIntro::start()
{
	json jboot = loadJson("data/boot.json");

	// Auto load some scenes
	std::vector< std::string > scenes_to_auto_load = jboot["intro_scenes"];
	for (auto& scene_name : scenes_to_auto_load) {
		dbg("Autoloading scene %s\n", scene_name.c_str());
		TEntityParseContext ctx;
		parseScene(scene_name, ctx);
	}

	Engine.getNavmeshes().buildNavmesh("data/navmeshes/test.bin");

	Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
	mouse->setLockMouse(true);
	ShowCursor(false);

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
	Engine.getGUI().activateWidget("ingame");
	Engine.getSound().setAmbientSound("../bin/data/sounds/ambient.ogg");

	return true;
}

bool CModuleMapIntro::stop()
{
	Engine.getEntities().destroyAllEntities();
	Engine.getCameras().deleteAllCameras();
	Engine.getIA().clearSharedBoards();
	Engine.getNavmeshes().destroyNavmesh();

	return true;
}

void CModuleMapIntro::update(float delta)
{
	VEC2 mouse = EngineInput.mouse()._position;

}

void CModuleMapIntro::render()
{
	// Render the grid
	cb_object.obj_world = MAT44::Identity;
	cb_object.obj_color = VEC4(1, 1, 1, 1);
	cb_object.updateGPU();

	auto solid = Resources.get("data/materials/solid.material")->as<CMaterial>();
	solid->activate();

	auto axis = Resources.get("axis.mesh")->as<CRenderMesh>();
	axis->activateAndRender();

}
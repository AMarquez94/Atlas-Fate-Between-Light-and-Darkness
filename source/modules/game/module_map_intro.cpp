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

bool CModuleMapIntro::start()
{
	json jboot = loadJson("data/boot.json");
	
	// Auto load some scenes
	std::vector< std::string > scenes_to_auto_load = jboot["light_scenes_test"];
	for (auto& scene_name : scenes_to_auto_load) {
		dbg("Autoloading scene %s\n", scene_name.c_str());
		TEntityParseContext ctx;
		parseScene(scene_name, ctx);
	}

	//// -------------------------------------------
	//if (!cb_camera.create(CB_CAMERA))
	//	return false;
	//// -------------------------------------------
	//if (!cb_object.create(CB_OBJECT))
	//	return false;

	//if (!cb_light.create(CB_LIGHT))
	//	return false;

	//cb_light.activate();
	//cb_object.activate();
	//cb_camera.activate();

	Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
	mouse->setLockMouse(true);
	ShowCursor(false);

	auto om = getObjectManager<CEntity>();
	om->forEach([](CEntity* e) {
		TMsgSceneCreated msg;
		CHandle h_e(e);
		h_e.sendMsg(msg);
	});

	return true;
}

bool CModuleMapIntro::stop()
{
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
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

bool CModuleMapIntro::start() {

#ifdef _DEBUG
    EngineScene.loadScene("scene_brian");
#else
    EngineScene.loadScene("scene_alex");
#endif

	Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
	mouse->setLockMouse(true);

	//CHandle h_camera = getEntityByName("TPCamera");
	//if (h_camera.isValid())
	//	Engine.getCameras().setDefaultCamera(h_camera);

	//h_camera = getEntityByName("main_camera");
	//if (h_camera.isValid())
	//	Engine.getCameras().setOutputCamera(h_camera);

	//auto om = getObjectManager<CEntity>();
	//om->forEach([](CEntity* e) {
	//	TMsgSceneCreated msg;
	//	CHandle h_e(e);
	//	h_e.sendMsg(msg);
	//});

	Engine.getGUI().activateWidget("ingame");
	Engine.getSound().setAmbientSound("../bin/data/sounds/ambient.ogg");

	return true;
}

bool CModuleMapIntro::stop() {

	return true;
}

void CModuleMapIntro::update(float delta) {


}

void CModuleMapIntro::render() {

	// Render the grid
	cb_object.obj_world = MAT44::Identity;
	cb_object.obj_color = VEC4(1, 1, 1, 1);
	cb_object.updateGPU();

	auto solid = Resources.get("data/materials/solid.material")->as<CMaterial>();
	solid->activate();

	auto axis = Resources.get("axis.mesh")->as<CRenderMesh>();
	axis->activateAndRender();

}
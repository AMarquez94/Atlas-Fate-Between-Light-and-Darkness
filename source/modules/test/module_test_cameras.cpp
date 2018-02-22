#include "mcv_platform.h"
#include "module_test_cameras.h"
#include "render/render_objects.h"
#include "render/texture/material.h"
#include "entity/entity_parser.h"
#include "components/comp_camera.h"

extern CCamera camera;
extern void registerMesh(CRenderMesh* new_mesh, const char* name);

struct TVtxPosClr {
	VEC3 pos;
	VEC4 color;
	TVtxPosClr(VEC3 new_pos, VEC4 new_color) : pos(new_pos), color(new_color) {}
};

// ---------------------------------------------------
CRenderMesh* createCurveMesh(const CCurve& curve, int nsteps) {
	CRenderMesh* mesh = new CRenderMesh;

	std::vector<TVtxPosClr> vtxs;
	VEC4 clr(1.f, 1.f, 1.f, 1.0f);
	for (int i = 0; i < nsteps; ++i) {
		vtxs.emplace_back(curve.evaluateAsCatmull((float)i / (float)nsteps), clr);
		vtxs.emplace_back(curve.evaluateAsCatmull((float)(i + 1) / (float)nsteps), clr);
	}

	if (!mesh->create(vtxs.data(), vtxs.size() * sizeof(TVtxPosClr), "PosClr", CRenderMesh::LINE_LIST))
		return nullptr;
	return mesh;
}

bool CModuleTestCameras::start()
{
	{
		TEntityParseContext ctx;
		parseScene("data/scenes/test_cameras.scene", ctx);
	}

	CHandle h_camera = getEntityByName("test_camera_flyover");
	Engine.getCameras().setDefaultCamera(h_camera);

	h_camera = getEntityByName("the_camera");
	Engine.getCameras().setOutputCamera(h_camera);

	_curve.addKnot(VEC3(-10, -3, 5));
	_curve.addKnot(VEC3(-8, 3, 5));
	_curve.addKnot(VEC3(-6, 3, 5));
	_curve.addKnot(VEC3(-4, -3, 5));
	_curve.addKnot(VEC3(-2, -3, 5));
	_curve.addKnot(VEC3(0, 3, 5));
	_curve.addKnot(VEC3(2, 3, 5));
	_curve.addKnot(VEC3(4, -3, 5));
	_curve.addKnot(VEC3(6, -3, 5));
	_curve.addKnot(VEC3(8, 3, 5));
	_curve.addKnot(VEC3(10, 3, 5));

	registerMesh(createCurveMesh(_curve, 100), "curve.mesh");

	return true;
}

void CModuleTestCameras::update(float delta)
{
	if (EngineInput['1'].getsPressed())
	{
		CHandle h_camera = getEntityByName("test_camera_orbit");
		Engine.getCameras().blendInCamera(h_camera, 1.f, CModuleCameras::EPriority::GAMEPLAY);
	}
	if (EngineInput['2'].getsPressed())
	{
		CHandle h_camera = getEntityByName("test_camera_fixed_A");
		Engine.getCameras().blendInCamera(h_camera, 1.f, CModuleCameras::EPriority::GAMEPLAY);
	}
	if (EngineInput['3'].getsPressed())
	{
		CHandle h_camera = getEntityByName("test_camera_fixed_B");
		Engine.getCameras().blendInCamera(h_camera, 1.f, CModuleCameras::EPriority::GAMEPLAY);
	}
	if (EngineInput['4'].getsPressed())
	{
		static Interpolator::TCubicInOutInterpolator interpolator;
		CHandle h_camera = getEntityByName("test_camera_fixed_C");
		Engine.getCameras().blendInCamera(h_camera, 1.f, CModuleCameras::EPriority::TEMPORARY, &interpolator);
	}
	if (EngineInput['5'].getsPressed())
	{
		CHandle h_camera = getEntityByName("test_camera_fixed_C");
		Engine.getCameras().blendOutCamera(h_camera, 0.f);
	}
}

void CModuleTestCameras::render()
{

	// Find the entity with name 'the_camera'
	CHandle h_e_camera = getEntityByName("the_camera");
	if (h_e_camera.isValid()) {
		CEntity* e_camera = h_e_camera;
		TCompCamera* c_camera = e_camera->get<TCompCamera>();
		assert(c_camera);
		activateCamera(*c_camera);
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
	auto curve = Resources.get("curve.mesh")->as<CRenderMesh>();
	curve->activateAndRender();
}

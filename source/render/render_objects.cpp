#include "mcv_platform.h"
#include "render_objects.h"

CRenderCte<CCteCamera> cb_camera("Camera");
CRenderCte<CCteObject> cb_object("Object");
CRenderCte<CCteLight>  cb_light("Light");

struct TVtxPosClr {
	VEC3 pos;
	VEC4 color;
	TVtxPosClr() {}
	TVtxPosClr(VEC3 new_pos, VEC4 new_color) : pos(new_pos), color(new_color) {}
};

// ---------------------------------------------------

CRenderMesh* createLineZ() {
	CRenderMesh* mesh = new CRenderMesh;
	// Axis aligned X,Y,Z of sizes 1,2,3
	float vertices[] =
	{
		0.0f, 0.0f, 0.0f,  1, 1, 1, 1,
		0.0f, 0.0f, 1.0f,  1, 1, 1, 1,
	};
	if (!mesh->create(vertices, sizeof(vertices), "PosClr", CRenderMesh::LINE_LIST))
		return nullptr;
	return mesh;
}

// ---------------------------------------------------
CRenderMesh* createAxis() {
	CRenderMesh* mesh = new CRenderMesh;
	// Axis aligned X,Y,Z of sizes 1,2,3
	float vertices[] =
	{
		0.0f, 0.0f, 0.0f,  1, 0, 0, 1,
		1.0f, 0.0f, 0.0f,  1, 0, 0, 1,
		0.0f, 0.0f, 0.0f,  0, 1, 0, 1,
		0.0f, 2.0f, 0.0f,  0, 1, 0, 1,
		0.0f, 0.0f, 0.0f,  0, 0, 1, 1,
		0.0f, 0.0f, 3.0f,  0, 0, 1, 1,
	};
	if (!mesh->create(vertices, sizeof(vertices), "PosClr", CRenderMesh::LINE_LIST))
		return nullptr;
	return mesh;
}

// ---------------------------------------------------
CRenderMesh* createUnitCircleXZ(int nsamples) {
	CRenderMesh* mesh = new CRenderMesh;

	std::vector< TVtxPosClr > vtxs;
	vtxs.resize(nsamples * 2);
	auto* v = vtxs.data();
	VEC4 clr(1, 1, 1, 1);
	float du = 2.0f * (float)(M_PI) / (float)(nsamples);
	VEC3 p = getVectorFromYaw(0.0f);
	for (int i = 1; i <= nsamples; ++i) {
		*v++ = TVtxPosClr(VEC3(p.x, 0.0f, p.z), clr);
		p = getVectorFromYaw(i * du);
		*v++ = TVtxPosClr(VEC3(p.x, 0.0f, p.z), clr);
	}
	assert(v == vtxs.data() + vtxs.size());
	if (!mesh->create(vtxs.data(), vtxs.size() * sizeof(TVtxPosClr), "PosClr", CRenderMesh::LINE_LIST))
		return nullptr;
	return mesh;
}

CRenderMesh* createGridXZ(int nsteps) {
	CRenderMesh* mesh = new CRenderMesh;

	std::vector<TVtxPosClr> vtxs;
	VEC4 clr2(0.5f, 0.5f, 0.5f, 1.0f);
	VEC4 clr1(0.25f, 0.25f, 0.25f, 1.0f);
	for (int i = -nsteps; i <= nsteps; ++i) {
		VEC4 clr = (i % 5) ? clr1 : clr2;
		float fi = (float)i;
		float fnsteps = (float)nsteps;
		vtxs.emplace_back(VEC3(fi, 0, fnsteps), clr);
		vtxs.emplace_back(VEC3(fi, 0, -fnsteps), clr);
		vtxs.emplace_back(VEC3(fnsteps, 0, fi), clr);
		vtxs.emplace_back(VEC3(-fnsteps, 0, fi), clr);
	}

	if (!mesh->create(vtxs.data(), vtxs.size() * sizeof(TVtxPosClr), "PosClr", CRenderMesh::LINE_LIST))
		return nullptr;
	return mesh;
}

// ---------------------------------------------------
CRenderMesh* createCameraFrustum() {
	CRenderMesh* mesh = new CRenderMesh;

	std::vector<TVtxPosClr> vtxs;
	VEC4 clr(1, 1, 1, 1);
	vtxs.emplace_back(VEC3(-1, -1, 0), clr);
	vtxs.emplace_back(VEC3(1, -1, 0), clr);
	vtxs.emplace_back(VEC3(-1, -1, 1), clr);
	vtxs.emplace_back(VEC3(1, -1, 1), clr);
	vtxs.emplace_back(VEC3(-1, 1, 0), clr);
	vtxs.emplace_back(VEC3(1, 1, 0), clr);
	vtxs.emplace_back(VEC3(-1, 1, 1), clr);
	vtxs.emplace_back(VEC3(1, 1, 1), clr);

	std::vector<uint16_t> idxs;
	for (int i = 0; i < 4; ++i) {
		// Lines along +x
		idxs.push_back(i * 2);
		idxs.push_back(i * 2 + 1);
		// Vertical lines
		idxs.push_back(i);
		idxs.push_back(i + 4);
	}
	idxs.push_back(0);
	idxs.push_back(2);
	idxs.push_back(1);
	idxs.push_back(3);
	idxs.push_back(4);
	idxs.push_back(6);
	idxs.push_back(5);
	idxs.push_back(7);
	if (!mesh->create(vtxs.data(), vtxs.size() * sizeof(TVtxPosClr), "PosClr"
		, CRenderMesh::LINE_LIST
		, idxs.data(), idxs.size() * sizeof(uint16_t), sizeof(uint16_t)
	))
		return nullptr;
	return mesh;
}

// ----------------------------------
// To render wired AABB's
CRenderMesh* createWiredUnitCube() {
	std::vector<TVtxPosClr> vtxs =
	{
		{ VEC3(-0.5f,-0.5f, -0.5f),  VEC4(1, 1, 1, 1) },    // 
	{ VEC3(0.5f,-0.5f, -0.5f),   VEC4(1, 1, 1, 1) },
	{ VEC3(-0.5f, 0.5f, -0.5f),  VEC4(1, 1, 1, 1) },
	{ VEC3(0.5f, 0.5f, -0.5f),   VEC4(1, 1, 1, 1) },    // 
	{ VEC3(-0.5f,-0.5f, 0.5f),   VEC4(1, 1, 1, 1) },    // 
	{ VEC3(0.5f,-0.5f, 0.5f),    VEC4(1, 1, 1, 1) },
	{ VEC3(-0.5f, 0.5f, 0.5f),   VEC4(1, 1, 1, 1) },
	{ VEC3(0.5f, 0.5f, 0.5f),    VEC4(1, 1, 1, 1) },    // 
	};
	const std::vector<uint16_t> idxs = {
		0, 1, 2, 3, 4, 5, 6, 7
		, 0, 2, 1, 3, 4, 6, 5, 7
		, 0, 4, 1, 5, 2, 6, 3, 7
	};
	CRenderMesh* mesh = new CRenderMesh;
	const int nindices = 8 * 3;
	if (!mesh->create(vtxs.data(), vtxs.size() * sizeof(TVtxPosClr), "PosClr"
		, CRenderMesh::LINE_LIST
		, idxs.data(), idxs.size() * sizeof(uint16_t), sizeof(uint16_t)
	))
		return nullptr;
	return mesh;
}


CRenderMesh* createCone(float hor_fov, float ver_fov, float dist, int steps, VEC4 clr) {
	CRenderMesh* mesh = new CRenderMesh;

	std::vector<TVtxPosClr> vtxs;

	vtxs.emplace_back(VEC3(0.f, 0.f, 0.f), clr);
	vtxs.emplace_back(VEC3(dist * sinf(-hor_fov / 2), 0.f, dist * cosf(-hor_fov / 2)), clr);
	vtxs.emplace_back(VEC3(0.f, 0.f, 0.f), clr);
	vtxs.emplace_back(VEC3(dist * sinf(hor_fov / 2), 0.f, dist * cosf(hor_fov / 2)), clr);

	float fovUnit = hor_fov / steps;

	for (int i = -steps / 2; i < steps / 2; i++) {
		vtxs.emplace_back(VEC3(dist * sinf(fovUnit * i), 0.f, dist * cosf(fovUnit * i)), clr);
		vtxs.emplace_back(VEC3(dist * sinf(fovUnit * (i + 1)), 0.f, dist * cosf(fovUnit * (i + 1))), clr);
	}

	vtxs.emplace_back(VEC3(0.f, 0.f, 0.f), clr);
	vtxs.emplace_back(VEC3(0.f, dist * sinf(-ver_fov / 2), dist * cosf(-ver_fov / 2)), clr);
	vtxs.emplace_back(VEC3(0.f, 0.f, 0.f), clr);
	vtxs.emplace_back(VEC3(0.f, dist * sinf(ver_fov / 2), dist * cosf(ver_fov / 2)), clr);

	fovUnit = ver_fov / steps;

	for (int i = -steps / 2; i < steps / 2; i++) {
		vtxs.emplace_back(VEC3(0.f, dist * sinf(fovUnit * i), dist * cosf(fovUnit * i)), clr);
		vtxs.emplace_back(VEC3(0.f, dist * sinf(fovUnit * (i + 1)), dist * cosf(fovUnit * (i + 1))), clr);
	}


	/*vtxs.emplace_back(VEC3(dist * cosf(-fov / 2), 0.f, dist * sinf(-fov / 2)), clr);
	vtxs.emplace_back(VEC3(dist * cosf(fov / 2), 0.f, dist * sinf(fov / 2)), clr);*/

	if (!mesh->create(vtxs.data(), vtxs.size() * sizeof(TVtxPosClr), "PosClr", CRenderMesh::LINE_LIST))
		return nullptr;
	return mesh;
}

// --------------------------
void registerMesh(CRenderMesh* new_mesh, const char* name) {
	new_mesh->setNameAndClass(name, getResourceClassOf<CRenderMesh>());
	Resources.registerResource(new_mesh);
}

bool createRenderObjects() {
	registerMesh(createAxis(), "axis.mesh");
	registerMesh(createGridXZ(20), "grid.mesh");
	registerMesh(createLineZ(), "line.mesh");
	registerMesh(createUnitCircleXZ(32), "circle_xz.mesh");
	registerMesh(createCameraFrustum(), "unit_frustum.mesh");
	registerMesh(createCone(deg2rad(179.f), deg2rad(89.f), 35.f, 10, VEC4(1.0f, 1.0f, 1.0f, 1.0f)), "cone_of_vision.mesh");
	registerMesh(createCone(deg2rad(35.f), deg2rad(45.f), 20.f, 10, VEC4(1.0f, 1.0f, 0.0f, 1.0f)), "cone_of_light.mesh");
	registerMesh(createWiredUnitCube(), "wired_unit_cube.mesh");
	return true;
}

void destroyRenderObjects() {
}

void activateCamera(const CCamera& camera) {
	cb_camera.camera_view = camera.getView();
	cb_camera.camera_proj = camera.getProjection();
	cb_camera.camera_pos = camera.getPosition();
	cb_camera.updateGPU();
}

void setWorldTransform(MAT44 new_matrix, VEC4 new_color) {
	cb_object.obj_world = new_matrix;
	cb_object.obj_color = new_color;
	cb_object.updateGPU();
}

void renderMesh(const CRenderMesh* mesh, MAT44 new_matrix, VEC4 color) {

	assert(mesh);
	auto vdecl = mesh->getVertexDecl();
	assert(vdecl);
	const char* tech_name = "solid.tech";
	if (vdecl->name == "PosNUv")
		tech_name = "textured.tech";
	else if (vdecl->name == "PosNUvUv")
		tech_name = "textured_bk.tech";
	else if (vdecl->name == "PosNUvSkin")
		tech_name = "solid_objs_skin.tech";
	else if (vdecl->name == "PosNUvTanSkin")
		tech_name = "solid_objs_skin.tech";

	auto prev_tech = CRenderTechnique::current;
	auto tech = Resources.get(tech_name)->as<CRenderTechnique>();
	tech->activate();
	
	setWorldTransform(new_matrix, color);
	mesh->activateAndRender();
	prev_tech->activate();
}

// ---------------------------------------------
void renderWiredAABB(const AABB& aabb, MAT44 world, VEC4 color) {
	// Accede a una mesh que esta centrada en el origen y
	// tiene 0.5 de half size
	auto mesh = Resources.get("wired_unit_cube.mesh")->as<CRenderMesh>();
	MAT44 unit_cube_to_aabb = MAT44::CreateScale(VEC3(aabb.Extents) * 2.f)
		* MAT44::CreateTranslation(aabb.Center)
		* world;
	renderMesh(mesh, unit_cube_to_aabb, color);
}
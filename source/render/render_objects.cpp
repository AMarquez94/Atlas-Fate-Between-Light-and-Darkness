#include "mcv_platform.h"
#include "render_objects.h"
#include "texture/texture.h"

CRenderCte<CCteCamera>  cb_camera("Camera");
CRenderCte<CCteObject>  cb_object("Object");
CRenderCte<CCteLight>   cb_light("Light");
CRenderCte<CCteGlobals> cb_globals("Globals");
CRenderCte<CCteBlur>    cb_blur("Blur");
CRenderCte<CCteGUI>     cb_gui("Gui");

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

// ----------------------------------
// Full screen quad to dump textures in screen
CRenderMesh* createUnitQuadXY() {
	const VEC4 white(1, 1, 1, 1);
	const std::vector<TVtxPosClr> vtxs = {
		{ VEC3(0, 0, 0), white }
		,{ VEC3(1, 0, 0), white }
		,{ VEC3(0, 1, 0), white }
		,{ VEC3(1, 1, 0), white }
	};
	CRenderMesh* mesh = new CRenderMesh;
	if (!mesh->create(vtxs.data(), vtxs.size() * sizeof(TVtxPosClr), "PosClr"
		, CRenderMesh::TRIANGLE_STRIP
	))
		return nullptr;
	return mesh;
}

// ----------------------------------
// Full screen quad to dump textures in screen
CRenderMesh* createUnitQuadPosXY() {
    const std::vector<VEC3> vtxs = {
        VEC3(0, 0, 0)
        , VEC3(1, 0, 0)
        , VEC3(0, 1, 0)
        , VEC3(1, 1, 0)
    };
    CRenderMesh* mesh = new CRenderMesh;
    if (!mesh->create(vtxs.data(), vtxs.size() * sizeof(VEC3), "Pos"
        , CRenderMesh::TRIANGLE_STRIP
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
	registerMesh(createCone(deg2rad(120.f), deg2rad(89.f), 35.f, 10, VEC4(1.0f, 1.0f, 1.0f, 1.0f)), "cone_of_vision_mimetic.mesh");
	registerMesh(createCone(deg2rad(120.f), deg2rad(89.f), 35.f, 10, VEC4(1.0f, 1.0f, 1.0f, 1.0f)), "cone_of_vision_patrol.mesh");
	registerMesh(createCone(deg2rad(35.f), deg2rad(45.f), 20.f, 10, VEC4(1.0f, 1.0f, 0.0f, 1.0f)), "cone_of_light.mesh");
	registerMesh(createWiredUnitCube(), "wired_unit_cube.mesh");
	registerMesh(createUnitQuadXY(), "unit_quad_xy.mesh");
    registerMesh(createUnitQuadPosXY(), "unit_quad_pos_xy.mesh");

	return true;
}

void destroyRenderObjects() {
}

void activateCamera(CCamera& camera, int width, int height) {

	assert(width > 0);
	assert(height > 0);
	camera.setViewport(0, 0, width, height);
	cb_camera.camera_view = camera.getView();
	cb_camera.camera_proj = camera.getProjection();
	cb_camera.camera_view_proj = camera.getViewProjection();
	cb_camera.camera_pos = camera.getPosition();
	cb_camera.camera_dummy1 = 1.f;
	cb_camera.camera_front = camera.getFront();
	cb_camera.camera_dummy2 = 0.f;
	cb_camera.camera_front = camera.getFront();
	cb_camera.camera_dummy2 = 0.f;
	cb_camera.camera_left = camera.getLeft();
	cb_camera.camera_dummy3 = 0.f;
	cb_camera.camera_up = camera.getUp();
	cb_camera.camera_dummy4 = 0.f;

	// To avoid converting the range -1..1 to 0..1 in the shader
	// we concatenate the view_proj with a matrix to apply this offset
	MAT44 mtx_offset = MAT44::CreateScale(VEC3(0.5f, -0.5f, 1.0f))
		* MAT44::CreateTranslation(VEC3(0.5f, 0.5f, 0.0f));
	cb_camera.camera_proj_with_offset = camera.getProjection() * mtx_offset;

	cb_camera.camera_zfar = camera.getZFar();
	cb_camera.camera_znear = camera.getZNear();
	cb_camera.camera_tan_half_fov = tan(camera.getFov() * 0.5f);
	cb_camera.camera_aspect_ratio = camera.getAspectRatio();

	cb_camera.camera_inv_resolution = VEC2(1.0f / (float)width, 1.0f / (float)height);
	
	// Simplify conversion from screen coords to world coords 
	MAT44 m = MAT44::CreateScale(-cb_camera.camera_inv_resolution.x * 2.f, -cb_camera.camera_inv_resolution.y * 2.f, 1.f)
		*MAT44::CreateTranslation(1, 1, 0)
		*MAT44::CreateScale(cb_camera.camera_tan_half_fov * cb_camera.camera_aspect_ratio, cb_camera.camera_tan_half_fov, 1.f)
		*MAT44::CreateScale(cb_camera.camera_zfar);
	
	// Now the transform local to world coords part
	// float3 wPos =
	//     CameraFront.xyz * view_dir.z
	//   + CameraLeft.xyz  * view_dir.x
	//   + CameraUp.xyz    * view_dir.y
	MAT44 mtx_axis = MAT44::Identity;
	mtx_axis.Forward(-camera.getFront());      // -getFront() because MAT44.Forward negates our input
	mtx_axis.Left(-camera.getLeft());
	mtx_axis.Up(camera.getUp());
	
	cb_camera.camera_screen_to_world = m * mtx_axis;
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

	const char* tech_name = nullptr;
	if (vdecl->name == "PosNUv")
		tech_name = "solid_objs.tech";
    else if (vdecl->name == "PosClr")
        tech_name = "solid.tech";
	else if (vdecl->name == "PosNUvUvT")
		tech_name = "solid_objs_uv2.tech";
	else if (vdecl->name == "PosNUvSkin")
		tech_name = "pbr_skin.tech";
	else if (vdecl->name == "PosNUvTanSkin")
		tech_name = "solid_objs_skin.tech";
    else {
        // Don't know how to render this type of vertex
        return;
    }

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

// ---------------------------------------------
void renderLine(VEC3 src, VEC3 dst, VEC4 color) {

	MAT44 world = MAT44::CreateLookAt(src, dst, VEC3(0, 1, 0)).Invert();
	float distance = VEC3::Distance(src, dst);
	world = MAT44::CreateScale(1, 1, -distance) * world;
	cb_object.obj_world = world;
	cb_object.obj_color = color;
	cb_object.updateGPU();

	auto mesh = Resources.get("line.mesh")->as<CRenderMesh>();
	mesh->activateAndRender();
}

// ---------------------------------------------
void renderFullScreenQuad(const std::string& tech_name, const CTexture* texture) {

	auto* tech = Resources.get(tech_name)->as<CRenderTechnique>();
	assert(tech);
	tech->activate();
	if (texture)
		texture->activate(TS_ALBEDO);
	auto* mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
	mesh->activateAndRender();
}

// ---------------------------------------------
bool createDepthStencil(
	const std::string& aname,
	int width, int height,
	DXGI_FORMAT format,
	// outputs
	ID3D11Texture2D** depth_stencil_resource,
	ID3D11DepthStencilView** depth_stencil_view,
	CTexture** out_ztexture
) {

	assert(format == DXGI_FORMAT_R32_TYPELESS
		|| format == DXGI_FORMAT_R24G8_TYPELESS
		|| format == DXGI_FORMAT_R16_TYPELESS
		|| format == DXGI_FORMAT_D24_UNORM_S8_UINT
		|| format == DXGI_FORMAT_R8_TYPELESS);

	// Crear un ZBuffer de la resolucion de mi backbuffer
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	// The format 'DXGI_FORMAT_D24_UNORM_S8_UINT' can't be binded to shader resource
	if (format != DXGI_FORMAT_D24_UNORM_S8_UINT)
		desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

	// SRV = Shader Resource View
	// DSV = Depth Stencil View
	DXGI_FORMAT texturefmt = DXGI_FORMAT_R32_TYPELESS;
	DXGI_FORMAT SRVfmt = DXGI_FORMAT_R32_FLOAT;       // Stencil format
	DXGI_FORMAT DSVfmt = DXGI_FORMAT_D32_FLOAT;       // Depth format

	switch (format) {
	case DXGI_FORMAT_R32_TYPELESS:
		SRVfmt = DXGI_FORMAT_R32_FLOAT;
		DSVfmt = DXGI_FORMAT_D32_FLOAT;
		break;
	case DXGI_FORMAT_R24G8_TYPELESS:
		SRVfmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		DSVfmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
		break;
	case DXGI_FORMAT_R16_TYPELESS:
		SRVfmt = DXGI_FORMAT_R16_UNORM;
		DSVfmt = DXGI_FORMAT_D16_UNORM;
		break;
	case DXGI_FORMAT_R8_TYPELESS:
		SRVfmt = DXGI_FORMAT_R8_UNORM;
		DSVfmt = DXGI_FORMAT_R8_UNORM;
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		SRVfmt = desc.Format;
		DSVfmt = desc.Format;
		break;
	default:
		fatal("Unsupported format creating depth buffer\n");
	}

	HRESULT hr = Render.device->CreateTexture2D(&desc, NULL, depth_stencil_resource);
	if (FAILED(hr))
		return false;
	setDXName(*depth_stencil_resource, aname.c_str());

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DSVfmt;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = Render.device->CreateDepthStencilView(*depth_stencil_resource, &descDSV, depth_stencil_view);
	if (FAILED(hr))
		return false;
	setDXName(*depth_stencil_view, (aname + "_DSV").c_str());

	if (out_ztexture) {
		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format = SRVfmt;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = desc.MipLevels;

		// Create the shader resource view.
		ID3D11ShaderResourceView* depth_resource_view = nullptr;
		hr = Render.device->CreateShaderResourceView(*depth_stencil_resource, &shaderResourceViewDesc, &depth_resource_view);
		if (FAILED(hr))
			return false;

		CTexture* ztexture = new CTexture();
		ztexture->setDXParams(width, height, *depth_stencil_resource, depth_resource_view);
		ztexture->setNameAndClass("Z" + aname, getResourceClassOf<CTexture>());
		Resources.registerResource(ztexture);
		setDXName(*depth_stencil_resource, (ztexture->getName() + "_DSR").c_str());
		setDXName(depth_resource_view, (ztexture->getName() + "_DRV").c_str());

		// The ztexture already got the reference
		depth_resource_view->Release();
		*out_ztexture = ztexture;
	}

	return true;
}
#include "mcv_platform.h"
#include "camera.h"

CCamera::CCamera() {
	
	type = PERSPECTIVE;

	setPerspective(deg2rad(60.f), 1.0f, 1000.f);
	lookAt(VEC3(1, 0, 0), VEC3(0, 0, 0), VEC3(0, 1, 0));
}

void CCamera::updateViewProj() {

  view_proj = view * proj;
}

// 
void CCamera::lookAt(VEC3 new_pos, VEC3 new_target, VEC3 new_up_aux) {
  
	pos = new_pos;
	target = new_target;
	up_aux = new_up_aux;

	front = (target - pos);
	front.Normalize();

	left = new_up_aux.Cross(front);
	left.Normalize();

	up = front.Cross(left);

	view = MAT44::CreateLookAt(pos, target, up_aux);
	updateViewProj();
}

void CCamera::setPerspective(float new_fov_vertical, float new_z_near, float new_z_far) {

	type = PERSPECTIVE;

	fov_vertical = new_fov_vertical;
	z_near = new_z_near;
	z_far = new_z_far;

	proj = MAT44::CreatePerspectiveFieldOfView(new_fov_vertical, aspect_ratio, new_z_near, new_z_far);
	updateViewProj();
}

void CCamera::setOrtographic(float ortosize, float new_z_near, float new_z_far) {

	type = ORTOGRAPHIC;

	z_near = new_z_near;
	z_far = new_z_far;
	orto_size = ortosize;

	proj = MAT44::CreateOrthographic(ortosize, ortosize, new_z_near, new_z_far);
	updateViewProj();
}

void CCamera::setViewport(int x0, int y0, int width, int height) {
	// save params
	viewport.x0 = x0;
	viewport.y0 = y0;
	viewport.width = width;
	viewport.height = height;

	aspect_ratio = (float)width / (float)height;

	if (type == CType::PERSPECTIVE)
		setPerspective(fov_vertical, z_near, z_far);
	else
		setOrtographic(orto_size, z_near, z_far);
}

bool CCamera::getScreenCoordsOfWorldCoord(VEC3 world_pos, VEC3* result) const {

	// It's also dividing by w  -> [-1..1]
	VEC3 pos_in_homo_space = VEC3::Transform(world_pos, getViewProjection());

	// Convert to 0..1 and then to viewport coordinates
	VEC3 pos_in_screen_space(
		viewport.x0 + (pos_in_homo_space.x + 1.0f) * 0.5f * viewport.width,
		viewport.y0 + (1.0f - pos_in_homo_space.y) * 0.5f * viewport.height,
		pos_in_homo_space.z
	);

	assert(result);
	*result = pos_in_screen_space;

	// Return true if the coord is inside the frustum
	return pos_in_homo_space.x >= -1.0f && pos_in_homo_space.x <= 1.0f
		&& pos_in_homo_space.y >= -1.0f && pos_in_homo_space.y <= 1.0f
		&& pos_in_homo_space.z >= 0.0f && pos_in_homo_space.z <= 1.0f
		;
}

